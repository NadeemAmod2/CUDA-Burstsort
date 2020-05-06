
#include "main.h"
using namespace std;

/* Constructors and Destructors */
Textfile::Textfile()
{
	text = 0;
	lines = 0;
	numberlines = 0;
}

Textfile::Textfile(wchar_t* file)
{
	text = 0;
	lines = 0;
	open(file);
}

Textfile::~Textfile()
{
	close();
}

/* Detects the encoding of a filebuffer if it is a text file */
/* Must ensure src is at least three bytes in size */
ENCODING Textfile::detectencoding(unsigned char* src)
{
	if (src[0]==0xEF && src[1]==0xBB && src[2]==0xBF)
		return UTF8;
	else if (src[0]==0xFF && src[1]==0xFE)
		return Unicode;
	else if (src[0]==0xFE && src[1]==0xFF)
		return UnicodeBE;
	else
		return ASCII;
}

/* Opens a file and returns a unicode string to the contents */
wchar_t* Textfile::open(wchar_t* filename)
{
	// Open file
	fstream myfile(filename, ios::binary || ios::in);
	if(myfile.fail()) {
		wcout << L"Couldn't open " << filename << endl;
		exit(1);
	}
	// Get file size
	myfile.seekg (0, ios::end);
	unsigned int length = myfile.tellg();
	myfile.seekg (0, ios::beg);
		
	// Read contents of the file, then close it
	char* filestring = new char[length + 2];
	myfile.read(filestring, length);	//reinterpret_cast<const char_t*>
	unsigned int bytesread = myfile.gcount();
	if (!bytesread) {
		wcout << L"File " << filename << L" is empty." << endl;
		exit(1);
	}
	myfile.close();
	// Make the file contents a null terminated string (Unicode)
	filestring[length] = (unsigned char) 0;
	filestring[length + 1] = (unsigned char) 0;
	// Detect encoding and if nessecary(*) convert it
	ENCODING filetype = detectencoding ((unsigned char*) filestring);
	// Turn this into a switch case arrangement
	if (filetype == Unicode)
		text = (wchar_t*) (filestring + 2);
	else if (filetype == ASCII)
		text = ASCIItoUnicode((unsigned char*) filestring, length);
	else if (filetype == UTF8)
		text = UTF8toUnicode((unsigned char*) (filestring + 3));
	else
		text = UnicodeBEtoUnicode((unsigned char*) (filestring + 2));
	return text;
}

/* Breaks the text file into lines if necessary and then returns how many lines there are */
/* Once you have called this function, you cannot get the file string. */
unsigned int Textfile::countlines()
{
	// if the file string was cleared then return 0 lines
	if (text == 0 && lines == 0) {
		numberlines = 0;
		return numberlines;
	}
	// if we need to split the file string into lines then do so
	if (lines == 0)
		splitlines();
	// return the number of lines
	return numberlines;
}

/* Returns a pointer to the string */
wchar_t* Textfile::getline(unsigned int linenumber) 
{
	// make sure we don't go out of bounds with the array
	if (linenumber < numberlines) 
		return lines[linenumber];
	else
		return 0;
}

/* Splits the file string into an array of lines */
/* Originally I did this using the C++ standard library (in this version of Burstsort) */
/* but it was too slow. So I rewrote the Textfile class instead to make it more "C++" */
void Textfile::splitlines() 
{
	// Count how many lines in the file
	numberlines = 0;
	for (unsigned int i = 0;;) {
		numberlines++;
		if (!text[i = nextline(text, i)]) break;
	}
	lines = new wchar_t*[numberlines];
	// Split into lines now
	int beforenextline = 0, linelen = 0;
	for (unsigned int i = 0, j = 0; j < numberlines; j++) {
		beforenextline = i;
		i = nextline(text, i);
		linelen = i - beforenextline - whatnewline(text, i);
		lines[j] = new wchar_t[linelen + 1];
		wcsncpy(lines[j], &text[beforenextline], linelen);
		lines[j][linelen] = 0;
	}
	// Free file string, don't neccesarily have to do this now, but it frees up some memory
	delete [] text;
	text = 0;
}

unsigned int Textfile::nextline(wchar_t* text, unsigned int i)
{
	while(text[i]!=0x0d && text[i]!=0x0a && text[i]!=0)
		i++;
	if ((text[i]==0x0d && text[i+1]==0x0a) || (text[i+1]==0x0d && text[i]==0x0a))
		i++;
	if (text[i])
		i++;
	return i;
}

unsigned int Textfile::whatnewline(wchar_t* f, int i)
{
	if (i > 1) {
		if (((f[i-1] == 0x0d) && (f[i-2] == 0x0a)) || ((f[i-2] == 0x0d) && (f[i-1] == 0x0a)))
			return 2;
	}
	if (i > 0) {
		if (f[i-1] == 0x0d || f[i-1] == 0x0a)
			return 1;
	}
	return 0;
}

/* Frees the memory and zeros the pointers */
void Textfile::close()
{
	if (text) {
		delete [] text;
		text = 0;
	}
	if (lines) {
		for (unsigned int i = 0; i < numberlines; i++)
			delete [] lines[i];
		delete [] lines;
		lines = 0;
	}
}

/* Returns a pointer to the string of the file */
wchar_t* Textfile::gettext()
{
	return text;
}

/* Converts from UTF8 to Unicode */
wchar_t* Textfile::UTF8toUnicode (unsigned char *utf8)
{
	int size = 0, index = 0, out_index = 0;
    wchar_t *out;
    unsigned char c;

    /* first calculate the size of the target string */
    c = utf8[index++];
    while(c) {
        if((c & 0x80) == 0) {
            index += 0;
        } else if((c & 0xe0) == 0xe0) {
            index += 2;
        } else {
            index += 1;
        }
        size += 1;
        c = utf8[index++];
    }   

    out = new wchar_t[size + 1];
    if (out == NULL)
        return NULL;
    index = 0;

    c = utf8[index++];
    while(c)
    {
        if((c & 0x80) == 0) {
            out[out_index++] = c;
        } else if((c & 0xe0) == 0xe0) {
            out[out_index] = (c & 0x1F) << 12;
           c = utf8[index++];
            out[out_index] |= (c & 0x3F) << 6;
           c = utf8[index++];
            out[out_index++] |= (c & 0x3F);
        } else {
            out[out_index] = (c & 0x3F) << 6;
           c = utf8[index++];
            out[out_index++] |= (c & 0x3F);
        }
        c = utf8[index++];
    }
    out[out_index] = 0;
    return out;
}

/* Converts from ASCII to Unicode */
wchar_t* Textfile::ASCIItoUnicode (unsigned char* ascii, int size)
{
	wchar_t* out;
	int index = 0;
    out = new wchar_t[size + 1];
	if (out==NULL)
		return NULL;
	while (ascii[index]) {
		// how to do the type conversion correctly??
		out[index] = ascii[index];
		index++;
	}
	out[index] = 0;
	delete [] ascii;
	return out;
}

/* Converts from Big-endian Unicode to regular Unicode */
wchar_t* Textfile::UnicodeBEtoUnicode (unsigned char* unicodebe)
{
	int index = 0;
	unsigned char temp;
	while (unicodebe[index] != 0 || unicodebe[index+1] != 0) {
		temp = unicodebe[index];
		unicodebe[index] = unicodebe[index + 1];
		unicodebe[index + 1] = temp;
		index += 2;
	}
	return (wchar_t*) unicodebe;
}