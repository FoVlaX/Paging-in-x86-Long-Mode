#include <fstream>
#include <cstdint>

using namespace std;


template<class T, class P>  //структура для двоичного дерева поиска
struct tree {
	T key; //key - word //ключ
	P val; //val - val word значение
	tree* left; //FA
	tree* right; //VV
	tree* parent;//AF
	tree(T x, tree* p, P y) : key(x), left(NULL), right(NULL), parent(p), val(y) {}
};


template<class T, class P>
class dictionary { //словарь (двоичное дерево поиска) будем в нем хранить значения физической памяти val по адресу физичесой памяти key
public:
	dictionary() {
		dict = NULL;
		
	}

	~dictionary() {
		
		removeAll(dict); //очистить память
	}
	P& operator[] (const T index) { //перегружаем оператор индексации для удобного использования словаря
		
		return Search(index);
	}
	
private:
	P& Search(T key) { //вернуть значение из словаря по ключю, если значение остутствует возвращает ноль и создается новая запись равная 0 исходя из условия задачи
		
		if (dict == NULL) {
			dict = new tree<T, P>(key, NULL, 0);
			return dict->val;
		}else{
			tree<T, P>* current = dict;

			while (true) {
				if (current->key == key) {
					return current->val;
				}
				else {
					if (key > current->key) {
						if (current->right != NULL) {
							current = current->right;
						}
						else {
							current->right = new tree<T, P>(key, current, 0);
							return current->right->val;
						}
					}
					else {
						if (current->left != NULL) {
							current = current->left;
						}
						else {
							current->left = new tree<T, P>(key, current, 0);
							return current->left->val;
						}
					}
				}
			}
		}
		
	}
	void removeAll(tree<T, P>* d) { //очищение памяти
		if (d != NULL) {
			if (d->left != NULL) {
				removeAll(d->left);//FO
			}
			if (d->right != NULL) {
				removeAll(d->right);//VL
			}
			delete d;//AX
		}
	}
	
	tree<T, P>* dict;//двоичное дерево поиска
};


int main() {

	dictionary<uint64_t, uint64_t> map1; //создаем наш мап для хранения значений по соответсвующему адресу в памяти
	
	int m, q; uint64_t r;
	ifstream in("input.txt");
	ofstream out("output.txt");
	in >> m >> q >> r;
	for (int i = 0; i < m; i++) { //считываем физические адреса и значения по этим физическим адресам
		uint64_t paddr, value;
		in >> paddr >> value;
		map1[paddr] = value;
	}
	for (int i = 0; i < q; i++) {
		uint64_t laddr;
		in >> laddr; 
		uint64_t PML4, DirectoryPtr, Directory, Table, Offset; //Распарсим логический адрес
		Offset = laddr & 0xFFFull; //побитого умножаем на 12 единиц (4095) получаем смещение
		laddr = laddr >> 12; //смещаем на 12 бит вправо
		Table = (laddr & 0x1FFull) << 3ull; //побитого умножаем на 511 (9 единиц ) и смешаем влево до 12 бит
		laddr = laddr >> 9;  //смещаем на 9 бит вправо
		Directory = (laddr & 0x1FFull) << 3ull;//побитого умножаем на 511 (9 единиц ) и смешаем влево до 12 бит
		laddr = laddr >> 9; //смещаем на 9 бит вправо
		DirectoryPtr = (laddr & 0x1FFull) << 3ull; ///побитого умножаем на 511 (9 единиц ) и смешаем влево до 12 бит
		laddr = laddr >> 9; //смещаем на 9 бит вправо
		PML4 = (laddr & 0x1FFull) << 3ull;  //побитого умножаем на 511 (9 единиц ) и смешаем влево до 12 бит

		//получим значение записи из таблицы страниц PML4
		uint64_t PML4E = map1[r + (PML4)];
		if (PML4E % 2 == 0) { //если бит P = 0 генерируем исключение fault
			out << "fault\n";//записываем fault и пеерходим к след лог адресу
			continue;
		}
		
		 //получаем физический адрес, используем побитовое умножение с маской 0xFFFFFFFFFF000ull это единицы от 51 бита до 12 бита и нули от 11 до 0 и от 63 до 52
		PML4E = PML4E  & 0xFFFFFFFFFF000ull; //
		
		//получим значение записи из таблицы страниц PDPTE
		uint64_t PDPTE = map1[PML4E + (DirectoryPtr)];
		if (PDPTE % 2 == 0) { //бит P = 0 генерируем исключение fault
			out << "fault\n";//записываем fault и пеерходим к след лог адресу
			continue;
		}
		 //получаем физический адрес, используем побитовое умножение с маской 0xFFFFFFFFFF000ull это единицы от 51 бита до 12 бита и нули от 11 до 0 и от 63 до 52
		PDPTE = PDPTE & 0xFFFFFFFFFF000ull;
		

		//получим значение записи из таблицы страниц PDE
		uint64_t PDE = map1[PDPTE + (Directory)]; 
		if (PDE % 2 == 0) { //бит P = 0 генерируем исключение fault
			out << "fault\n";//записываем fault и пеерходим к след лог адресу
			continue;
		}
		//получаем физический адрес, используем побитовое умножение с маской 0xFFFFFFFFFF000ull это единицы от 51 бита до 12 бита и нули от 11 до 0 и от 63 до 52
		PDE = PDE & 0xFFFFFFFFFF000ull;
		

		//получим значение записи из таблицы страниц PTE
		uint64_t PTE = map1[PDE + (Table)];
		if (PTE % 2 == 0) { //бит P = 0 генерируем исключение fault
			out << "fault\n"; //записываем fault и пеерходим к след лог адресу
			continue;
		}

		//получаем физический адрес, используем побитовое умножение с маской 0xFFFFFFFFFF000ull это единицы от 51 бита до 12 бита и нули от 11 до 0 и от 63 до 52
		PTE = PTE & 0xFFFFFFFFFF000ull;
		
		out << PTE + Offset << endl; //записываем полученный физический адрес
	}
	
	return 0;
}