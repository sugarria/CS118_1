void dostuff(int); /* function prototype */
char* fileType(char*);
char* currentTime();
char* lastModified(char*);
char* serverIP();
int contentLength(char*);
char* parseMessage(char*);

void debug();

typedef struct HeaderData {
	char* URL;
	char* currentTime;
	long server;
	char* lastModified;
	int length;
	char* type;
	char* content;
}HeaderData, HeaderData_p;

HeaderData* initializeHeaderData(char*);
char* constructHeader(HeaderData*);