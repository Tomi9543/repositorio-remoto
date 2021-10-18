#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <mysql/mysql.h> 
#include <mysql.h>


typedef struct{
	char Usuario[20];
	char Password [20];
	int id;
}Jugador;

typedef struct{
	int id;
	char fecha[10];
	int duracion_MINUTOS;
	char ganador[20];
}Partida;

//Declaramos parametros
char peticion[512];
char respuesta[512];

int Usuario_Registrado(MYSQL *conn, char nombre_Usuario[20]){
	//Retorna 1 si el usuario ya esta en la base de datos
	//Retorna 0 si no esta registrado
	int registrado;
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char consulta[80];
	strcpy (consulta, "SELECT jugador.username from (jugador) where jugador.username = '");
	strcat (consulta, nombre_Usuario);	//añade al final de la consulta el nombre pasado por parametro
	strcat (consulta, "'");
	
	int err;
	err = mysql_query(conn, consulta);
	
	resultado = mysql_store_result(conn);
	fila = mysql_fetch_row(resultado);
	
	if (fila == NULL) //no se ha encontrado al usuario
		return 0;
	else
		registrado = 1;	//Usuario ya registrado;
		
	mysql_close(conn);
	exit(0);
	return registrado;
}

int Password_Check(MYSQL *conn, char password[20], char nombre_Usuario[20]){
	//Retorna 1 si la contraseña y el usuario pasados por parametro coinciden	
	int checked;
	MYSQL_RES*resultado;
	MYSQL_ROW fila;
	char consulta[80];
	strcpy (consulta, "SELECT jugador.username from (jugador) where jugador.username = '");
	strcat (consulta, nombre_Usuario);		//pasado por parametro
	strcat (consulta, "' AND jugador.password = '");
	strcat (consulta, password);			//pasado por parametro
	strcat (consulta, "'");
	
	int err;
	err = mysql_query(conn, consulta);
	
	resultado = mysql_store_result(conn);
	fila = mysql_fetch_row(resultado);
	
	if(fila == NULL)
		checked = 0;	//No se ha pasado ningun dato, esta vacio
	
	else
		checked = 1;	//Si la contraseña y el usuario SÍ coinciden
	
	return checked;
}

void Registrar_Usuario(MYSQL *conn, char nombre_Usuario[20], char password[20], int id){
	
	MYSQL_RES *resultado;
	char consulta[80];
	consulta[0] = '\0';	//la primera posicion es para identificar el codigo
	sprintf(consulta, "INSERT into jugador VALUES ('%s', '%s', %d)", nombre_Usuario, password, id);
	
	int err;
	err = mysql_query(conn, consulta);
		
	mysql_close(conn);
	exit(0);
}

int Asignar_ID(MYSQL *conn ){
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char request[100];
	int total = 0;
	
	int err;
	err = mysql_query(conn, request);		//Hacemos la consulta
	
	strcpy(request, "Select * from (jugador)");
	resultado = mysql_store_result(conn);
	fila=mysql_fetch_row(resultado);
	
	if (fila != NULL){
		while (fila!=NULL){
			total = total+1;
			fila = mysql_fetch_row (resultado);
		}
		return total;
	}
	else
		return 0;
	mysql_close(conn);
	exit(0);
}

int JugadoresQueJugaronTalDIA (MYSQL *conn, char day[20] ){
	
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char request [100];
	int num_Jugadores = 0;
	
	strcpy(request, "Select distinct jugador.username from ( ) where partida.fecha = '");
	strcat(request, day);
	strcat(request, "' and participacion.id_jugador = jugador.id and participacion.id_partida = partida.id");
	
	int err;
	err = mysql_query(conn, request);
	
	resultado = mysql_store_result(conn);
	fila = mysql_fetch_row(resultado);
	while (fila != NULL){
		if (fila != NULL){
			num_Jugadores++;
		}
		else
			printf("Ha habido un error");
	return num_Jugadores;
	}
	mysql_close(conn);
	exit(0);	
}

int main(int argc, char *argv[]){
	
	MYSQL *conn;
	conn = mysql_init(NULL);
	int sock_conn, sock_listen, ret;
	int PORT = 9010;
	struct sockaddr_in serv_adr;
	char day[20];
	
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	
	//Fem bind al port
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// establecemos el puerto de escucha
	serv_adr.sin_port = htons(PORT); //cambiamos numero de puerto donde escuchamos
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	if (listen(sock_listen, 3) < 0)
		printf("Error en el Listen"); //hasta 3 peticiones en cola
	conn =mysql_real_connect(conn, "localhost", "root","mysql", "juego",0, NULL, 0);
	if (conn == NULL){
		printf("Error al crear la conexion");
		exit(1);
	}
	
	if (conn == NULL){
		printf ("Error al inicializar");
		exit(1);
	}
	
	//Bucle infinito para recibir peticiones
	int end = 0;
	while (end == 0){
		printf("Escuchando\n");
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		
		// Ahora recibimos la petici?n
		ret = read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		
		// Tenemos que añadirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		
		
		printf ("Peticion: %s\n",peticion);
		
		// vamos a ver que quieren
		char *p = strtok( peticion, "/");
		int codigo =  atoi (p);
		char nombre_Usuario[20];
		char password[20];
		
		if (codigo == 1){ 		//Peticion de login
			p = strtok(NULL, "/");
			strcpy(nombre_Usuario, p);
			p = strtok(NULL,"/");
			strcpy(password, p);
			
			int registrado = Usuario_Registrado(conn,nombre_Usuario);
			if (registrado == 0){
				strcpy(respuesta, "1");	//User not registered
				printf ("Usuario no registrado\n");
			}
			else{
				int checked = Password_Check(conn, password, nombre_Usuario);
				if (checked == 1){
					strcpy(respuesta,"2");	//Login succesful
					printf("Login succesful");
				}
				else{
					strcpy(respuesta, "3");	//Password wrong
					printf("Contraseña incorrecta");
				}
			}
		}
		
		else if (codigo == 2){	//Peticion de registro
			p = strtok(NULL, "/");
			strcpy(nombre_Usuario, p);
			p = strtok(NULL,"/");
			strcpy(password, p);
			int num_jugadores;
			
			int registrado = Usuario_Registrado(conn, nombre_Usuario);
			if (registrado = 1){	//Usuario regsitrado
				strcpy(respuesta, "Usuario existente");
				printf("%s\n", respuesta);
			}
			else {			//USUARIO NO REGISTRADO
				int num_jugadores = Asignar_ID(conn) +1;
				Registrar_Usuario(conn, nombre_Usuario, password, num_jugadores);
				strcpy(respuesta, "Usuario registrado correctamente");
				printf (respuesta);				
			}
		}
		
		else if (codigo == 0){	//Peticion de desconexion
			end = 1;	//Fin del bucle
		}
		
		else if (codigo == 4){	//Consulta los jugadores que han jugado con IronMan
			
		}
		else if	(codigo == 5){	//Consulta los jugadores que han ganado a IronMan
			
		}
		else if (codigo == 6){	//Consulta los jugadores que jugaron el dia 29/05/2021
			strcpy(day, "29/05/2021");
			int numero = JugadoresQueJugaronTalDIA (conn, day);
			strcpy(respuesta, numero);
			
		}
		if (codigo != 0)
		{
			printf ("Respuesta: %s\n", respuesta);
			// Enviamos respuesta
			write (sock_conn,respuesta, strlen(respuesta));
		}
		// Se acabo el servicio para este cliente
		close(sock_conn); 
	}	
}
	
