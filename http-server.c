/*
 * tcp-recver.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

static void die(const char *s) { perror(s); exit(1); }


int main(int argc, char **argv)
{

	if(signal(SIGPIPE,SIG_IGN)==SIG_ERR) die("signal() failed");

	
    if (argc != 5) {
        fprintf(stderr, "usage: %s <server-port> <web_root> <mdb-lookup-host> <mdb-lookup-port>\n", argv[0]);
        exit(1);
    }


    unsigned short servport = atoi(argv[1]);
    const char *webroot = argv[2];
    const char *lclhost = argv[3];
    unsigned short lclport = atoi(argv[4]);
    const char *form = 
		"<h1>mdb-lookup</h1>\n"
		"<p>\n"
		"<form method=GET action=/mdb-lookup>\n"
		"lookup: <input type=text name=key>\n"
		"<input type=submit>\n"
		"</form>\n"
		"<p>\n";

    // Create a listening socket (also called server socket) 

    int servsock;
    if ((servsock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("socket failed");

    // Create a backend client socket
    int mdbsock;
    if ((mdbsock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	    die("socket failed");

    // Construct front address structure

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // any network interface
    servaddr.sin_port = htons(servport);

    // Construct local address structure

    struct hostent *he;
    if((he = gethostbyname(lclhost)) == NULL){
	    die("gethostbyname failed");
    }

    char *serverIP = inet_ntoa(*(struct in_addr *) he->h_addr);

    struct sockaddr_in mdbaddr;
    memset(&mdbaddr, 0, sizeof(mdbaddr));
    mdbaddr.sin_family = AF_INET;
    mdbaddr.sin_port = htons(lclport);
    mdbaddr.sin_addr.s_addr = inet_addr(serverIP);; 

    //connect to mdb-lookup
    if (connect(mdbsock, (struct sockaddr *) &mdbaddr, sizeof(mdbaddr))<0)
	    die("connect failed");
    
    //wrap mdbsock
    FILE *mdbfp = fdopen(mdbsock, "rb");
    if (mdbfp == NULL)
	    die("mdb socket file failed");

    // Bind to the local address
    if (bind(servsock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        die("bind failed");

    // Start listening for incoming connections

    if (listen(servsock, 5) < 0)
        die("listen failed");

    int clntsock;
    socklen_t clntlen;
    struct sockaddr_in clntaddr;

    while (1) {

        // Accept an incoming connection

        clntlen = sizeof(clntaddr); // initialize the in-out parameter

        if ((clntsock = accept(servsock,
                        (struct sockaddr *) &clntaddr, &clntlen)) < 0)//cln return the actual byte tht listen read
            die("accept failed");

        // accept() returned a connected socket (also called client socket)
        // and filled in the client's address into clntaddr
	//wrap up new server FILE *
	FILE *sockfp = fdopen(clntsock, "r");
	if (sockfp == NULL)
		die("socket file failed");

	//FGET GET()
	char requestLine[1000]; 
	if(fgets(requestLine, sizeof(requestLine), sockfp) == NULL){
		fclose(sockfp);
		close(clntsock);
		//robust server connection
		continue;
	}
	char *token_separators = "\t \r\n"; 
	char *method = strtok(requestLine, token_separators);
	char *requestURI = strtok(NULL, token_separators);
	char *httpVersion = strtok(NULL, token_separators);
	char cpy[1000];
	snprintf(cpy, sizeof(cpy), "%s \"%s\" %s", method, requestURI, httpVersion);

	FILE *fp;
	char wrngMsg[1000];
	char crtMsg[1000];
	char *rc;
	char key[3] = "..";
	
	if ( strcmp(method, "GET") || !(strcmp(httpVersion, "HTTP/1.0") || strcmp(httpVersion, "HTTP/1.1"))){
		snprintf(wrngMsg, sizeof(wrngMsg), "HTTP/1.0 501 Not Implemented\r\n\r\n"
				"<html><body><h1>501 Not Implemented</h1></body></html>");
		if((send(clntsock, wrngMsg, strlen(wrngMsg), 0)) != strlen(wrngMsg)) {
				die("send");
				}
		rc = "501 Not Implemented";
		fprintf(stdout, "%s %s %s\n", inet_ntoa(clntaddr.sin_addr), cpy, rc);
			
	}else if(strstr(requestURI, key) || requestURI[0] != '/'){
		snprintf(wrngMsg, sizeof(wrngMsg), "HTTP/1.0 400 Bad Request\r\n\r\n"
				"<html><body><h1>400 Bad Request</h1></body></html>");
		if((send(clntsock, wrngMsg, strlen(wrngMsg), 0)) != strlen(wrngMsg)) {
				die("send");
				}
		rc = "400 Bad Request";
		fprintf(stdout, "%s %s %s\n", inet_ntoa(clntaddr.sin_addr), cpy, rc);
	}else{
		snprintf(crtMsg, sizeof(crtMsg), "HTTP/1.0 200 OK\r\n\r\n");
		rc = "200 OK";
	
		if(!strcmp(requestURI, "/mdb-lookup")){
			if((send(clntsock, crtMsg, strlen(crtMsg), 0)) != strlen(crtMsg)) {
				die("send");
				}
			fprintf(stdout, "%s %s %s\n", inet_ntoa(clntaddr.sin_addr), cpy, rc);

			//send form to browser
			if(send(clntsock, form, strlen(form), 0) != strlen(form))
				die("form send fail");

		}else if(strstr(requestURI, "/mdb-lookup?key=")){
			if((send(clntsock, crtMsg, strlen(crtMsg), 0)) != strlen(crtMsg)) {
				die("send");
				}
			char *token_separate = "=";
			strtok(requestURI, token_separate);
			char *searchedKey = strtok(NULL, token_separate);
			fprintf(stdout, "looking up:[%s] %s %s %s\n", searchedKey, inet_ntoa(clntaddr.sin_addr), cpy, rc);
			strcat(searchedKey, "\n");
			
			//send searched word to mdb-lookup-server
			if(send(mdbsock, searchedKey, strlen(searchedKey), 0) != strlen(searchedKey))
				die("send searchedKey wrong");
			
			//send form to browser
			if(send(clntsock, form, strlen(form), 0) != strlen(form))
				die("form send fail");
			
			//Table formatting
			char *table = "<p><table border>\n";
			char *tableEnd = "</table>\n</body></html>";
			char *tableWhite = "<tr><td>";
			char *tableYellow = "<tr><td bgcolor=yellow>"; 

			//sending table
			if(send(clntsock, table, strlen(table), 0) != strlen(table))
				die("table head fail");
			
			char mdbResult[1000];
			int counter = 1;
			while(fgets(mdbResult, sizeof(mdbResult), mdbfp) != NULL){
				if(mdbResult[0] == '\n' || mdbResult[0] == '\r'){
					break;
				}
				strcat(mdbResult, "\n");
				if (counter++ % 2 == 1){
					if(send(clntsock, tableWhite, strlen(tableWhite), 0) != strlen(tableWhite))
						die("table White fail");
					if(send(clntsock, mdbResult, strlen(mdbResult), 0) != strlen(mdbResult))
						die("Result send 1 fail");
				}else{
					if(send(clntsock, tableYellow, strlen(tableYellow), 0) != strlen(tableYellow))
						die("table White fail");
					if(send(clntsock, mdbResult, strlen(mdbResult), 0) != strlen(mdbResult))
						die("Result send 2 fail");
				}
			}

			if(send(clntsock, tableEnd, strlen(tableEnd), 0) != strlen(tableEnd))
				die("Table End fail");
			
		}else{
			char fullUrl[1000];
			strcpy(fullUrl, webroot);
			strcat(fullUrl, requestURI);

			if(fullUrl[strlen(fullUrl)-1] == '/'){
				strcat(fullUrl, "index.html");
			}

			struct stat st;
			if(stat(fullUrl, &st) != 0){
				rc = "404 Not Found";
 				snprintf(wrngMsg, sizeof(wrngMsg), "HTTP/1.0 404 Not Found\r\n\r\n"
						"<html><body><h1>404 Not Found</h1></body></html>");
				fprintf(stdout, "%s %s %s\n", inet_ntoa(clntaddr.sin_addr), cpy, rc);
				if(send(clntsock, wrngMsg, strlen(wrngMsg), 0) != strlen(wrngMsg))
					die("not found send wrong");
				fclose(sockfp);
				close(clntsock);
				continue;
			}else if(S_ISDIR(st.st_mode)){
				strcat(fullUrl, "/index.html");
			}

        		if ((fp = fopen(fullUrl, "rb")) == NULL){
				rc = "404 Not Found";
 				snprintf(wrngMsg, sizeof(wrngMsg), "HTTP/1.0 404 Not Found\r\n\r\n"
						"<html><body><h1>404 Not Found</h1></body></html>");
				fprintf(stdout, "%s %s %s\n", inet_ntoa(clntaddr.sin_addr), cpy, rc);
				if(send(clntsock, wrngMsg, strlen(wrngMsg), 0) != strlen(wrngMsg))
					die("not found send wrong");
				fclose(sockfp);
				close(clntsock);
				continue;			
			}
		
			if((send(clntsock, crtMsg, strlen(crtMsg), 0)) != strlen(crtMsg)) {
				die("send");
				}
			fprintf(stdout, "%s %s %s\n", inet_ntoa(clntaddr.sin_addr), cpy, rc);
			char buf[4096] = {0};
			size_t result;	
			while((result = fread(buf, 1, sizeof(buf), fp))>0){
				if(send(clntsock, buf, result, 0) != result)
			       		die("send content");
			}
	
			if (ferror(fp)){	
				die("sockfp");
			}

			fclose(fp);
		}

	}
	fclose(sockfp);
        close(clntsock);
    }
    fclose(mdbfp);
    close(mdbsock);
}
