typedef struct HeaderData {
	char* URL;
	char* currentTime;
	char* lastModified;
	int length;
	char* type;
}HeaderData, HeaderData_p;

void dostuff(int); /* function prototype */
char* fileType(char*);
char* currentTime();
char* lastModified(char*);
char* serverIP();
int contentLength(char*);
char* parseMessage(char*);
void sendFile(int, FILE*);

void debug();

HeaderData* initializeHeaderData(char*);
char* constructHeader(HeaderData*);