#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <string>
#include <regex>

#define VERSION "0.1.12"

using namespace std;

struct RGB
{
	int r;
	int g;
	int b;
};

char hexes[6] = { 'a', 'b', 'c', 'd', 'e', 'f' };
char const *opts = "hvV:m:";
const option long_opts[] = {
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'v' },
	{ "value", 1, 0, 'V' },
	{ "mode", 1, 0, 'm' },
	{ 0, 0, 0, 0 }
};

template <class T>
bool arrayHasElement(T *arr, int arrSize, T elem)
{
	for (int i = 0; i < arrSize; i++)
	{
		if (arr[i] == elem)
		{
			return true;
		}
	}

	return false;
}

string removeStrInStr(string str, string other)
{
	return regex_replace(str, regex(other), "");
}

vector<string> splitByLength(string str, int length)
{
	vector<string> strings;
	for (int i = 0; i < str.length(); i += length)
	{
		strings.push_back(str.substr(i, length));
		//printf("%s\n", strings[i / length].c_str());
	}
	return strings;
}

int strToInt(string str)
{
	bool isNumber = true;
	bool isHex = true;

	str = removeStrInStr(str, "0x");
	str = removeStrInStr(str, "#");
	
	for(char& ch : str)
	{
		if (!isdigit(ch))
		{
			if (!arrayHasElement<char>(hexes, 6, tolower(ch)))
			{
				isHex = false;
			}
			else
			{
				isNumber = false;
			}
		}
	}

	int result = 0;
	if (isNumber)
	{
		result = stoi(str, nullptr, 10);
	}
	else if (isHex)
	{
		result = stoi(str, nullptr, 16);
	}

	return result;
}

void usage(char *name)
{
	printf("usage: %s [--help] [--value...] [--mode] [text]\n", name);
	printf("\t-h, --help\t\tthis message\n");
	printf("\t-v, --value=<value>\tRGB, HEX or HSV value\n");
	printf("\t-m, --mode=<mode>\tRGB, HEX or HSV mode (1, 2, 3)\n");
	//printf("\t--help this message\n");
	//printf("\t--value=value RGB, HEX or HSV value\n");
	//printf("\t--mode=mode RGB, HEX or HSV mode (1, 2, 3)\n\n");
	printf("Examples\n");
	printf("\t\"%s -v 255 -v 255 -v 0 -m 1\" will return code yellow\n", name);
	printf("\t\"%s -v ffff00 -m 2 AnyText\" will return yellow colored \"AnyText\"\n", name);
}

void HSVtoRGB(double H, double S, double V, RGB &rgb)
{
	if(H < 0 || H > 360 || 
		 S < 0 || S > 100 || 
		 V < 0 || V > 100)
	{
	  printf("The given HSV values are not in valid range");
	  return;
	}
	double s = S / 100;
	double v = V / 100;
	double C = s * v;
	double X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
	double m = v - C;
	double r, g, b;

	if(H >= 0 && H < 60)
	{
	  r = C, g = X, b = 0;
	}
	else if(H >= 60 && H < 120)
	{
	  r = X, g = C, b = 0;
	}
	else if(H >= 120 && H < 180)
	{
	  r = 0, g = C, b = X;
	}
	else if(H >= 180 && H < 240)
	{
	  r = 0, g = X, b = C;
	}
	else if(H >= 240 && H < 300)
	{
    r = X, g = 0, b = C;
	}
	else
	{
    r = C, g = 0,b = X;
	}

	rgb.r = (r + m) * 255;
	rgb.g = (g + m) * 255;
	rgb.b = (b + m) * 255;
}

int main(int argc, char **argv)
{
	string progName(argv[0]);
	int optIdx, c, mode = 0;
	vector<int> values;
	string posHex;
	
	while ((c = getopt_long(argc, argv, opts, long_opts, &optIdx)) != -1)
	{
		switch(c)
		{
			case (int)'h':
			{
				usage(argv[0]);
				return 0;
			}
			case (int)'v':
			{
				printf("Color2Ansi %s\n", VERSION);
				return 0;
			}
			case (int)'V':
			{
				if (values.size() > 3)
				{
					printf("The -v option must be 3 or less times\n");
					return 1;
				}

				string str = removeStrInStr(optarg, "0x");
				str = removeStrInStr(str, "#");

				smatch sm;
				regex_match(str, sm, regex("^([A-Fa-f0-9]{3,6})$"));

				if (sm.size() > 0)
				{
					string result = sm[0];
					if (result.length() >= 3)
					{
						posHex = result;
					}
				}

				values.push_back(strToInt(str));
				//printf("option 'v' selected: %s\n", optarg);
				break;
			}
			case (int)'m':
			{
				mode = strToInt(optarg);
				//printf("option 'm' selected: %s\n", optarg);
				break;
			}
			default:
			{
				printf("%s -h to get help\n", argv[0]);
				return 1;
			}
		}
	}

	if (mode == 0 && values.size() > 0)
	{
		if (values.size() == 3)
		{
			mode = 2;
		}
		else if (values.size() == 1)
		{
			mode = 1;
		}
	}

	argc -= optind;
	argv += optind;

	RGB rgb;

	switch (mode)
	{
		case 1:
		{
			rgb.r = values[0];
			rgb.g = values[1];
			rgb.b = values[2];
			
			break;
		}
		case 2:
		{
			vector<string> strs = splitByLength(posHex, posHex.length() == 6 ? 2 : 1);
			if (strs.size() < 3)
			{
				printf("Invalid syntax, need RRGGBB or 0xRRGGBB or #RRGGBB or\n");
				printf("                     RGB or 0xRGB or #RGB\n");
				return 1;
			}

			if (strs[0].size() == 1) strs[0].insert(0, 1, strs[0][0]);
			if (strs[1].size() == 1) strs[1].insert(0, 1, strs[1][0]);
			if (strs[2].size() == 1) strs[2].insert(0, 1, strs[2][0]);

			rgb.r = strToInt(strs[0]);
			rgb.g = strToInt(strs[1]);
			rgb.b = strToInt(strs[2]);

			break;
		}
		case 3:
		{
			HSVtoRGB(values[0], values[1], values[2], rgb);
			
			break;
		}
		default:
		{
			//printf("Invalid mode %d\n", mode);
			printf("%s -h to get help\n", progName.c_str());
			return 1;
		}
	}

	if (argc > 0)
	{
		printf("\x1b[38;2;%d;%d;%dm%s\n", rgb.r, rgb.g, rgb.b, argv[0]);
	}
	else
	{
		printf("\\x1b[38;2;%d;%d;%dm\n", rgb.r, rgb.g, rgb.b);
	}
	
	/*if (argc == 1)
	{
		printf("Invalid syntax, need %s <r> <g> <b> [str] or\n", argv[0]);
		printf("                     %s <rgb> [str]\n", argv[0]);
		return 1;
	}

	if (argc == 2 || argc == 3)
	{
		if (argc == 3)
		{
			argv[4] = argv[2];
			argc = 5;
		}
		else 
		{
			argc = 4;
		}
		string str = removeStrInStr(argv[1], "0x");
		str = removeStrInStr(str, "#");
		vector<string> strs = splitByLength(str, 2);
		if (strs.size() < 3 || strs[2].length() < 2)
		{
			printf("Invalid syntax, need RRGGBB or 0xRRGGBB or #RRGGBB\n");
			return 1;
		}
		else
		{
			char *cstr = new char[strs[0].length() + 1];
			strcpy(cstr, strs[0].c_str());
			argv[1] = cstr;
			
			cstr = new char[strs[1].length() + 1];
			strcpy(cstr, strs[1].c_str());
			argv[2] = cstr;

			cstr = new char[strs[2].length() + 1];
			strcpy(cstr, strs[2].c_str());
			argv[3] = cstr;
		}
	}
	
	int r = strToInt(argv[1]);
	int g = strToInt(argv[2]);
	int b = strToInt(argv[3]);

	if (argc > 4)
	{
		printf("\x1b[38;2;%d;%d;%dm%s\n", r, g, b, argv[4]);
	}
	else
	{
		printf("\\x1b[38;2;%d;%d;%dm\n", r, g, b);
	}*/
	
	return 0;
}

//"\x1b[38;2;0;255;0m"
