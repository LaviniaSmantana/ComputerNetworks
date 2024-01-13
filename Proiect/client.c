#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#define MAX_BUFFER_SIZE 5000

extern int errno;
int port;


void optiuni_meniu() {
    printf("\nMeniu:\n1. Vizualizeaza anunturile\n2. Adauga anunt\n3. Iesire\n");
}

// login din lista de membri
int conectare (int sd)
{
    	char username[20];
	char *parola;
	char utilizatori[10] = "conturi";  // conturi aflate pe Whitelist
	char linie[100];
	FILE *fp;
	char cont_gasit[2];
   

    	//citim si trimitem usernameul
    	printf("[client] Introduceti username-ul: ");
    	fflush(stdout);
    
    	if(read(0, username, 20) < 0 )
    	{
    		perror ("[client] Eroare la citirea username-ului.\n");
		return 0;
    	}
    	printf("[client] Username-ul a fost introdus cu succes!\n");

	if(send(sd, username, sizeof(username), 0) == -1)
	{
		perror ("[client] Eroare.\n");
		return 0;
	}

    	// citim si trimitem parola
	// folosim getpass pentru a transmite parola securizata
    	parola = getpass("[client] Introduceti o parola: ");  
	printf("[client] Ai introdus parola cu succes!\n");

	if(send(sd, parola, sizeof(parola), 0) == -1)
	{
		perror("[client] Eroare.\n");
		return 0;
	}
	
	if(read(sd, cont_gasit, sizeof(cont_gasit)) < 0)
	{
		perror("[client] Eroare.\n");
		return 0;
	}
	
	if(cont_gasit[0] == '1'){
		printf("[client] Te-ai conectat cu succes!\n");
		return 1;
	}
	else {
		perror("[client] Contul introdus nu se afla in lista de membri.\n");
		return 0;
	}
	
}


int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char msg[100];		// mesajul trimis

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[client] Eroare la socket().\n");
      return errno;
    }
  

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  
  server.sin_family = AF_INET;     /* familia socket-ului */
  
  server.sin_addr.s_addr = inet_addr(argv[1]);   /* adresa IP a serverului */
  
  server.sin_port = htons(port);    /* portul de conectare */
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
  
 	// se realizeaza login-ul cu username si parola
	if(conectare(sd) == 0)
	{
		return 0;
	}
	
	optiuni_meniu(); 
	
    	while(1)
    	{

        	char option[2];
        	printf("\n[client] Selectati o optiune din Meniu: ");
        	fflush(stdout);
        	
        	if(read(0, option, 2) < 0 )
    		{
    			perror ("[client] Eroare la citirea optiunii selectate.\n");
			return 0;
    		}
        
        	if(send(sd, option, sizeof(option), 0) < 0) {
        		perror("[client] Eroare la trimiterea optiunii.\n");
        		return 0;
       	 }
        
        	if(option[0] == '1')
        	{
        		// Vizualizare anunturi
        		char buffer[MAX_BUFFER_SIZE];
              		memset(buffer, '\0', sizeof(buffer));
              
              		if(recv(sd, buffer, sizeof(buffer), 0) < 0)
			{
				perror("[client] Eroare la citirea anunturilor.\n");
				return 0;
			}
			printf("\n%s", buffer);
        	}
        	else if(option[0] == '2') {
        		// Adauga anunt
                	char title[20];
                	char details[100];
                	
                	memset(title, '\0', sizeof(title));
			memset(details, '\0', sizeof(details));
                
                	//citim si trimitem titlul
    			printf("[client] Introduceti titlul anuntului: ");
    			fflush(stdout);
    
    			if(read(0, title, 20) < 0 )
    			{
    				perror ("[client] Eroare la citirea titlului.\n");
				return 0;
    			}
    			printf("[client] Titlul a fost introdus cu succes!\n");

			if(send(sd, title, sizeof(title), 0) == -1)
			{
				perror ("[client] Eroare.\n");
				return 0;
			}

              		//citim si trimitem detaliile
    			printf("[client] Introduceti detaliile anuntului: ");
    			fflush(stdout);
    
    			if(read(0, details, 100) < 0 )
    			{
    				perror ("[client] Eroare la citirea detaliilor.\n");
				return 0;
    			}
    			printf("[client] Detaliile au fost introduse cu succes!\n");

			if(send(sd, details, sizeof(details), 0) == -1)
			{
				perror ("[client] Eroare.\n");
				return 0;
			}
        	}
        	else if(option[0] == '3') {
        		
        		printf("[client] Te-ai deconectat.\n");
        		return 0;
        	}
        	else {
        		printf("[client] Comanda invalida. Incercati din nou!\n");
        	}
        } // end while
        close(sd);
}



