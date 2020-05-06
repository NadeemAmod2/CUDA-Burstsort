
enum ENCODING {ASCII, Unicode, UnicodeBE, UTF8};

using namespace std;

class Textfile {
private:
	ENCODING detectencoding (unsigned char*);
	static wchar_t* UTF8toUnicode (unsigned char*);
	static wchar_t* ASCIItoUnicode (unsigned char*, int);
	static wchar_t* UnicodeBEtoUnicode (unsigned char*);
	void splitlines();
	unsigned int nextline(wchar_t*, unsigned int);
	unsigned int whatnewline(wchar_t*, int);
	wchar_t* text;
	wchar_t** lines;
	unsigned int numberlines;
public:
	Textfile();
	Textfile(wchar_t*);
	~Textfile();
	wchar_t* open(wchar_t*);
	unsigned int countlines();
	void close();
	wchar_t* gettext();
	wchar_t* getline(unsigned int);
};
