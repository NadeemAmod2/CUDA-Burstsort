
#include "main.h"
using namespace std;

char Minuscule::alphabetsize()
{
	return 26 + 26 + 10 + 1 + 1;	// 'A'~'Z',  'a'~'z',  '0'~'9', 'space',  extra
}

bool Minuscule::isvalid(wchar_t* string)
{
	for (unsigned int i = 0; i < wcslen(string); i++) {
		if (!(string[i] >= L'a' && string[i] <= L'z' ||
			string[i] >= L'A' && string[i] <= L'Z' ||
			string[i] >= L'0' && string[i] <= L'9' ||
			string[i] == L' '))
			return false;
	}
	return true;
}

char Minuscule::nodetochar(unsigned int node)
{
	if (node > 0 && node <= 26) return (char)(node - 1 + L'A');
	if (node > 26 && node <= 52) return (char)(node - 27 + L'a');
	if (node > 52 && node <= 62) return (char)(node - 53 + L'0');
	return L' ';
}

unsigned int Minuscule::chartonode(char symbol)
{
	if (symbol >= L'A' && symbol <= L'Z') return (unsigned int)symbol - L'A' + 1;
	if (symbol >= L'a' && symbol <= L'z') return (unsigned int)symbol - L'a' + 27;
	if (symbol >= L'0' && symbol <= L'9') return (unsigned int)symbol - L'0' + 53;
	return (unsigned int)alphabetsize() - 1;
}
