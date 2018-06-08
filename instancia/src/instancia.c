#include "instancia.h"

int main(void) {
	//Creo archivo de log
	logInstancia = log_create("log_Instancia.log", "instancia", true,
			LOG_LEVEL_TRACE);
	log_trace(logInstancia, "Inicio el proceso instancia \n");

	//Conecto instancia con coordinador
	conectarInstancia();

	//Quedo a la espera de solicitudes
	recibirSolicitudes = true;
	while (recibirSolicitudes) {
		gestionarSolicitudes(socketCoordinador, (void*) procesarPaquete,
				logInstancia);
	}

	//Termina esi
	log_trace(logInstancia, "Termino el proceso instancia \n");

	//Destruyo archivo de log
	log_destroy(logInstancia);

	return EXIT_SUCCESS;
}

/*-------------------------Conexion-------------------------*/
void conectarInstancia() {
	//Leo la configuracion del esi
	t_config* configInstancia = leerConfiguracion();

	//Setteo las variables de configuracion
	char * coordinadorIP = config_get_string_value(configInstancia,
			"COORDINADOR_IP");
	int coordinadorPuerto = config_get_int_value(configInstancia,
			"COORDINADOR_PUERTO");
	algoritmoReemplazo = config_get_string_value(configInstancia,
			"ALGORITMO_REEMPLAZO");
	puntoMontaje = config_get_string_value(configInstancia, "PUNTO_MONTAJE");
	char * nombreInstancia = config_get_string_value(configInstancia,
			"NOMBRE_INSTANCIA");
	intervaloDump = config_get_int_value(configInstancia, "INTERVALO_DUMP");

	//Conecto al coordinador
	socketCoordinador = conectarCliente(coordinadorIP, coordinadorPuerto,
			INSTANCIA);

	enviarNombreInstancia(socketCoordinador, nombreInstancia);

	//Destruyo la configuracion
	config_destroy(configInstancia);
}

t_config* leerConfiguracion() {
	t_config* configInstancia = config_create(RUTA_CONFIG);
	if (!configInstancia) {
		log_error(logInstancia, "Error al abrir la configuracion \n");
		exit(EXIT_FAILURE);
	}
	return configInstancia;
}

/*-------------------------Procesamiento paquetes-------------------------*/
void procesarPaquete(t_paquete * unPaquete, int * client_socket) {
	switch (unPaquete->codigoOperacion) {
	case ENVIAR_INFO_INSTANCIA:
		procesarEnviarInfoInstancia(unPaquete);
		break;
	case SET:
		procesarSet(unPaquete, *client_socket);
		break;
	case SET_DEFINITIVO:
		procesarSetDefinitivo(unPaquete, *client_socket);
		break;
	case GET:
		procesarGet(unPaquete, *client_socket);
		break;
	case COMPACTAR:
		procesarCompactacion(unPaquete, *client_socket);
		break;
	case SOLICITAR_VALOR:
		procesarSolicitudValor(unPaquete, *client_socket);
		break;
	default:
		break;
	}
	destruirPaquete(unPaquete);
}

void procesarEnviarInfoInstancia(t_paquete * unPaquete) {
	t_infoInstancia * info = recibirInfoInstancia(unPaquete);

	//Setteo tam de entrada y cantidad
	cantEntradas = info->cantEntradas;
	tamanioEntrada = info->tamanioEntrada;

	//Creo el espacio de almacenamiento
	crearStorage();

	//Creo bitMap del storage
	crearBitMap();

	//Creo tabla de entradas
	crearTablaEntradas();

	//Verifico que no tenga archivos anteriores
	recuperarInformacionDeInstancia();

	//Creo el hilo para hacer el dump
	crearAlmacenamientoContinuo();

	//Libero memoria
	free(info);

}

void procesarSet(t_paquete * unPaquete, int client_socket) {
	t_claveValor * claveValor = recibirSet(unPaquete);

	int respuesta = agregarValorAClave(claveValor->clave, claveValor->valor);

	switch (respuesta) {
	case ENTRADA_INEXISTENTE:
		enviarRespuesta(client_socket, ERROR_CLAVE_NO_IDENTIFICADA);
		break;
	case CANTIDAD_INDEX_LIBRES_INEXISTENTES:
		enviarRespuesta(client_socket, ERROR_ESPACIO_INSUFICIENTE);
		break;
	default:
		enviarRespuesta(OK, client_socket);
		break;
	}

	free(claveValor->clave);
	free(claveValor->valor);
	free(claveValor);

}

void procesarSetDefinitivo(t_paquete * unPaquete, int client_socket) {
	t_claveValor * claveValor = recibirSetDefinitivo(unPaquete);

	int respuesta = agregarValorAClave(claveValor->clave, claveValor->valor);

	if (respuesta == CANTIDAD_INDEX_LIBRES_INEXISTENTES) {
		if (string_equals_ignore_case(algoritmoReemplazo, "CIRC")) {
			algoritmoReemplazoCircular(claveValor->clave, claveValor->valor);
		}

		if (string_equals_ignore_case(algoritmoReemplazo, "LRU")) {
			algoritmoReemplazoLeastRecentlyUsed(claveValor->clave,
					claveValor->valor);
		}

		if (string_equals_ignore_case(algoritmoReemplazo, "BSU")) {
			algoritmoReemplazoBiggestSpaceUsed(claveValor->clave,
					claveValor->valor);
		}
	}

	free(claveValor->clave);
	free(claveValor->valor);
	free(claveValor);

	enviarRespuesta(OK, client_socket);

}

void procesarGet(t_paquete * unPaquete, int client_socket) {
	char * clave = recibirGet(unPaquete);

	void * resultado = buscarValorSegunClave(clave);

	if (resultado == NULL) {
		agregarClave(clave);
	}

	enviarRespuesta(client_socket, OK);

	if (resultado != NULL)
		free(resultado);
	free(clave);
}

void procesarCompactacion(t_paquete * unPaquete, int client_socket) {
	compactar();
	enviarCompactacion(client_socket);
}

void procesarSolicitudValor(t_paquete * unPaquete, int client_socket) {
	char * clave = recibirSolicitudValor(unPaquete);

	t_tabla_entradas * respuesta = buscarEntrada(clave);

	if (respuesta == NULL) {
		enviarRespSolicitudValor(client_socket, false, NULL);
		free(clave);
		return;
	}

	enviarRespSolicitudValor(client_socket, true, respuesta->entrada);

	free(respuesta);
	free(clave);
}

/*-------------------------Tabla de entradas-------------------------*/
void crearTablaEntradas(void) {
	tablaEntradas = list_create();
}

void destruirTablaEntradas(void) {
	void eliminarEntrada(t_tabla_entradas * entrada) {
		free(entrada);
	}

	list_destroy_and_destroy_elements(tablaEntradas, (void *) eliminarEntrada);
}

t_tabla_entradas * buscarEntrada(char * clave) {
	bool esEntradaBuscada(t_tabla_entradas * entrada) {
		return string_equals_ignore_case(entrada->clave, clave);
	}

	t_tabla_entradas * registroEntrada = list_find(tablaEntradas,
			(void*) esEntradaBuscada);

	return registroEntrada;
}

void agregarClave(char * clave) {
	t_tabla_entradas * registroEntrada = malloc(sizeof(t_tabla_entradas));

	strncpy(registroEntrada->clave, clave, sizeof(registroEntrada->clave) - 1);

	registroEntrada->tamanio = 0;

	registroEntrada->indexComienzo = -1;

	registroEntrada->tiempoReferenciado = 0;

	list_add(tablaEntradas, registroEntrada);
}

void eliminarClave(char * clave) {
	bool esEntradaBuscada(t_tabla_entradas * entrada) {
		return string_equals_ignore_case(entrada->clave, clave);
	}

	t_tabla_entradas * entradaBuscada = list_remove_by_condition(tablaEntradas,
			(void*) esEntradaBuscada);

	if (entradaBuscada != NULL) {
		if (entradaBuscada->tamanio != 0) {
			int i;

			int cantidadEntradasABorar = entradasNecesariaParaUnTamanio(
					entradaBuscada->tamanio);

			for (i = 0; i < cantidadEntradasABorar; i++)
				liberarIndex(entradaBuscada->indexComienzo + i);
		}
		free(entradaBuscada);
	}
}

void mostrarTablaEntradas(void) {
	int i;

	printf("Clave			Tamanio			1°Entrada \n");
	printf("----------------------------------------------------------\n");

	for (i = 0; i < tablaEntradas->elements_count; i++) {
		t_tabla_entradas * entrada = list_get(tablaEntradas, i);
		printf("%s			%d			%d \n", entrada->clave, entrada->tamanio,
				entrada->indexComienzo);
	}
	printf("\n");
}

int agregarValorAClave(char * clave, void * valor) {
	t_tabla_entradas * registroEntrada = buscarEntrada(clave);

	if (registroEntrada == NULL)
		return ENTRADA_INEXISTENTE;

	int tamValor = string_length(valor);

	int index = -1;

	void * respuesta = guardarEnStorage(valor, &index);

	if (respuesta == NULL) {
		return CANTIDAD_INDEX_LIBRES_INEXISTENTES;
	} else {

		registroEntrada->entrada = respuesta;

		registroEntrada->tamanio = tamValor;

		registroEntrada->indexComienzo = index;

		return 0;
	}
}

void * buscarValorSegunClave(char * clave) {
	t_tabla_entradas * entrada = buscarEntrada(clave);

	if (entrada == NULL)
		return NULL;

	char * respuesta = malloc(entrada->tamanio + 1);

	memcpy(respuesta, entrada->entrada, entrada->tamanio);

	respuesta[entrada->tamanio] = '\0';

	return respuesta;
}

t_tabla_entradas * buscarEntradaSegunIndex(int index) {
	bool esEntradaBuscada(t_tabla_entradas * entrada) {
		return (entrada->indexComienzo == index);
	}

	t_tabla_entradas * registroEntrada = list_find(tablaEntradas,
			(void*) esEntradaBuscada);

	return registroEntrada;
}

void mostrarEntrada(char * clave) {
	t_tabla_entradas * entrada = buscarEntrada(clave);

	printf("Clave: %s \n", entrada->clave);

	char * respuesta = malloc(entrada->tamanio + 1);

	memcpy(respuesta, entrada->entrada, entrada->tamanio);

	respuesta[entrada->tamanio] = '\0';

	printf("Valor: %s \n", respuesta);

	printf("Tamanio: %d \n\n", entrada->tamanio);

	free(respuesta);
}

/*-------------------------BitMap del Storage-------------------------*/
void crearBitMap(void) {
	bitMap = malloc(sizeof(bool) * cantEntradas);

	liberarBitMap();
}

void destruirBitMap(void) {
	free(bitMap);
}

void liberarBitMap(void) {
	for (int i = 0; i < cantEntradas; i++)
		liberarIndex(i);
}

void llenarBitMap(void) {
	for (int i = 0; i < cantEntradas; i++)
		ocuparIndex(i);
}

void liberarIndex(int index) {
	if (index + 1 <= cantEntradas) {
		bitMap[index] = false;
	} else {
		log_error(logInstancia,
				"No se puede liberar el index %d ya que no existe \n", index);
	}
}

void ocuparIndex(int index) {
	if (index + 1 <= cantEntradas) {
		bitMap[index] = true;
	} else {
		log_error(logInstancia,
				"No se puede ocuapar el index %d ya que no existe \n", index);
	}
}

int buscarIndexLibre(void) {
	int index = 0;

	while (bitMap[index]) {
		index++;
	}

	if (index < 99)
		bitMap[index] = false;

	return index;
}

void mostrarBitmap(void) {
	printf("Index			Ocupado \n");
	printf("-------------------------------\n");
	int i;
	for (i = 0; i < cantEntradas; i++)
		printf("%d			%d \n", i, bitMap[i]);
	printf("\n");
}

int buscarCantidadIndexLibres(int cantidad) {
	bool loEncontre = false;
	int candidato;
	int contador;
	int i;

	for (i = 0; !loEncontre && i < cantEntradas; i++) {
		if (!bitMap[i]) {
			candidato = i;
			contador = 1;

			while (contador < cantidad && (i + 1) < cantEntradas
					&& !bitMap[i + 1]) {
				i++;
				contador++;
			}

			if (contador == cantidad)
				loEncontre = true;
		}
	}

	if (!loEncontre)
		candidato = CANTIDAD_INDEX_LIBRES_INEXISTENTES;

	return candidato;
}

int cantidadIndexLibres(void) {
	int i;
	int cantidad = 0;
	for (i = 0; i < cantEntradas; i++) {
		if (!bitMap[i])
			cantidad++;
	}
	return cantidad;
}

/*-------------------------Storage-------------------------*/
void crearStorage(void) {
	storage = malloc(cantEntradas * tamanioEntrada);
}

void destruirStorage(void) {
	free(storage);
}

void * guardarEnStorage(void * valor, int * index) {
	int tamValor = strlen(valor);

	int entradasNecesaria = entradasNecesariaParaUnTamanio(tamValor);

	*index = buscarCantidadIndexLibres(entradasNecesaria);

	if ((*index) != CANTIDAD_INDEX_LIBRES_INEXISTENTES) {

		return guardarEnStorageEnIndex(valor, *index);

	} else {
		return NULL;
	}
}

void * guardarEnStorageEnIndex(void * valor, int index) {
	int tamValor = strlen(valor);

	int entradasNecesaria = entradasNecesariaParaUnTamanio(tamValor);

	int i;

	for (i = 0; i < entradasNecesaria; i++)
		ocuparIndex(index + i);

	memcpy(storage + (index * tamanioEntrada), valor, tamValor);

	return storage + (index * tamanioEntrada);
}

void compactar(void) {
	int i;
	int primeraEntradaLibre;

	for (i = 0; i < cantEntradas; i++) {
		if (!bitMap[i]) {

			primeraEntradaLibre = i;

			for (; i < cantEntradas && !bitMap[i]; i++)
				;

			if (i == cantEntradas)
				break;

			int indexOcupado = i;

			t_tabla_entradas * entrada = buscarEntradaSegunIndex(i);

			int cantidadEntradas = entrada->tamanio / tamanioEntrada;

			if (entrada->tamanio % tamanioEntrada != 0)
				cantidadEntradas++;

			entrada->indexComienzo = primeraEntradaLibre;

			void * valor = buscarValorSegunClave(entrada->clave);

			entrada->entrada = guardarEnStorageEnIndex(valor,
					primeraEntradaLibre);

			free(valor);

			for (int i2 = 0; i2 < cantidadEntradas; i2++) {
				ocuparIndex(primeraEntradaLibre);
				liberarIndex(indexOcupado);
				primeraEntradaLibre++;
				indexOcupado++;
			}
		}

	}
}

/*-------------------------Dump-------------------------*/
void dump(void) {

	mkdir(puntoMontaje, 0777);

	void almacenarEnMemoriaSecundaria(t_tabla_entradas * registroEntrada) {
		char * rutaArchivo = string_new();
		string_append(&rutaArchivo, puntoMontaje);
		string_append(&rutaArchivo, "/");
		string_append(&rutaArchivo, registroEntrada->clave);

		FILE* file = fopen(rutaArchivo, "w+b");

		void * valor = buscarValorSegunClave(registroEntrada->clave);

		fwrite(valor, sizeof(void), registroEntrada->tamanio, file);

		fclose(file);

		free(rutaArchivo);
		free(valor);
	}

	list_iterate(tablaEntradas, (void*) almacenarEnMemoriaSecundaria);
}

void almacenamientoContinuo(void) {
	while (true) {
		sleep(intervaloDump);
		dump();
	}
}

void crearAlmacenamientoContinuo(void) {
	pthread_t threadAlmacenamientoContinuo;

	if (pthread_create(&threadAlmacenamientoContinuo, NULL,
			(void*) almacenamientoContinuo, NULL)) {
		perror("Error el crear el thread almacenamientoContinuo.");
		exit(EXIT_FAILURE);
	}
}

void recuperarInformacionDeInstancia(void) {
	t_list * listaArchivos = listarArchivosDeMismaCarpeta(puntoMontaje);

	if (listaArchivos == NULL)
		return;

	void guardarArchivoEnEstructurasAdministrativas(char * rutaArchivo) {
		size_t tamArch;
		FILE * archivofd;

		void * archivo = abrirArchivo(rutaArchivo, &tamArch, &archivofd);

		char ** spliteado = string_split(rutaArchivo, "/");

		int i;

		for (i = 0; spliteado[i] != NULL; i++)
			;

		agregarClave(spliteado[i - 1]);
		agregarValorAClave(spliteado[i - 1], archivo);

		for (i = 0; spliteado[i] != NULL; ++i) {
			free(spliteado[i]);
		}
		free(spliteado[i]);
		free(spliteado);

		munmap(archivo, tamArch);
		fclose(archivofd);

	}

	list_iterate(listaArchivos,
			(void*) guardarArchivoEnEstructurasAdministrativas);

	list_destroy_and_destroy_elements(listaArchivos, (void *) free);
}

/*-------------------------Algoritmos de reemplazo-------------------------*/
void algoritmoReemplazoCircular(char * clave, void * valor) {
	t_list * entradasAtomicas = ordenarEntradasAtomicasParaCircular();

	eliminarEntradasParaReemplazo(entradasAtomicas, valor);
}

void algoritmoReemplazoBiggestSpaceUsed(char * clave, void * valor) {
	t_list * entradasAtomicas = ordenarEntradasAtomicasParaBSU();

	eliminarEntradasParaReemplazo(entradasAtomicas, valor);
}

void algoritmoReemplazoLeastRecentlyUsed(char * clave, void * valor) {
	t_list * entradasAtomicas = ordenarEntradasAtomicasParaLRU();

	eliminarEntradasParaReemplazo(entradasAtomicas, valor);
}

t_list * ordenarEntradasAtomicasParaCircular(void) {
	t_list * entradasAtomicas = filtrarEntradasAtomicas();

	bool ordenarMenorMayorSegunIndex(t_tabla_entradas * entrada,
			t_tabla_entradas * entradaMenor) {
		return entrada->indexComienzo < entradaMenor->indexComienzo;
	}

	list_sort(entradasAtomicas, (void*) ordenarMenorMayorSegunIndex);

	bool mayoresAEntradaAReemplazar(t_tabla_entradas * registroTabla) {

		return (registroTabla->indexComienzo >= entradaAReemplazar);
	}

	t_list * mayoresAReemplazar = list_filter(entradasAtomicas,
			(void*) mayoresAEntradaAReemplazar);

	bool menoresAEntradaAReemplazar(t_tabla_entradas * registroTabla) {

		return (registroTabla->indexComienzo < entradaAReemplazar);
	}

	t_list * menoresAReemplazar = list_filter(entradasAtomicas,
			(void*) menoresAEntradaAReemplazar);

	list_add_all(mayoresAReemplazar, menoresAReemplazar);

	list_destroy(entradasAtomicas);
	list_destroy(menoresAReemplazar);

	return mayoresAReemplazar;

}

t_list * ordenarEntradasAtomicasParaBSU(void) {
	t_list * entradasAtomicas = filtrarEntradasAtomicas();

	bool ordenarMenorMayorSegunTamanio(t_tabla_entradas * entrada,
			t_tabla_entradas * entradaMayor) {

		if (entrada->tamanio == entradaMayor->tamanio) {
			t_list * listaDesempate = desempate(entrada, entradaMayor);
			t_tabla_entradas * primera = list_get(listaDesempate, 1);
			bool resultado = string_equals_ignore_case(primera->clave,
					entradaMayor->clave);
			list_destroy(listaDesempate);
			return resultado;
		}

		return entrada->tamanio > entradaMayor->tamanio;
	}

	list_sort(entradasAtomicas, (void*) ordenarMenorMayorSegunTamanio);

	return entradasAtomicas;
}

t_list * ordenarEntradasAtomicasParaLRU(void) {
	t_list * entradasAtomicas = filtrarEntradasAtomicas();

	bool ordenarMenorMayorSegunTiempoReferenciado(t_tabla_entradas * entrada,
			t_tabla_entradas * entradaMayor) {

		if (entrada->tiempoReferenciado == entradaMayor->tiempoReferenciado) {
			t_list * listaDesempate = desempate(entrada, entradaMayor);
			t_tabla_entradas * primera = list_get(listaDesempate, 1);
			bool resultado = string_equals_ignore_case(primera->clave,
					entradaMayor->clave);
			list_destroy(listaDesempate);
			return resultado;
		}

		return entrada->tiempoReferenciado > entradaMayor->tiempoReferenciado;
	}

	list_sort(entradasAtomicas,
			(void*) ordenarMenorMayorSegunTiempoReferenciado);

	return entradasAtomicas;
}

void eliminarEntradasParaReemplazo(t_list * entradasAtomicas, void * valor) {
	int entradasNecesarias = entradasNecesariaParaUnTamanio(
			string_length(valor));

	int i = 0;

	for (int a = cantidadIndexLibres();
			a < entradasNecesarias && a < entradasAtomicas->elements_count;
			a++, i++) {
		t_tabla_entradas * entrada = list_get(entradasAtomicas, i);
		eliminarClave(entrada->clave);
		entrada = list_get(entradasAtomicas, i + 1);
		if (entrada != NULL)
			entradaAReemplazar = entrada->indexComienzo;
	}

	list_destroy(entradasAtomicas);

}

t_list * desempate(t_tabla_entradas * entrada, t_tabla_entradas * entrada2) {
	t_list * entradasEmpatadas = list_create();

	list_add(entradasEmpatadas, entrada);
	list_add(entradasEmpatadas, entrada2);

	bool ordenarMenorMayorSegunIndex(t_tabla_entradas * entrada,
			t_tabla_entradas * entradaMenor) {
		return entrada->indexComienzo < entradaMenor->indexComienzo;
	}

	list_sort(entradasEmpatadas, (void*) ordenarMenorMayorSegunIndex);

	bool mayoresAEntradaAReemplazar(t_tabla_entradas * registroTabla) {

		return (registroTabla->indexComienzo >= entradaAReemplazar);
	}

	t_list * mayoresAReemplazar = list_filter(entradasEmpatadas,
			(void*) mayoresAEntradaAReemplazar);

	bool menoresAEntradaAReemplazar(t_tabla_entradas * registroTabla) {

		return (registroTabla->indexComienzo < entradaAReemplazar);
	}

	t_list * menoresAReemplazar = list_filter(entradasEmpatadas,
			(void*) menoresAEntradaAReemplazar);

	list_add_all(mayoresAReemplazar, menoresAReemplazar);

	list_destroy(entradasEmpatadas);
	list_destroy(menoresAReemplazar);

	return mayoresAReemplazar;
}

/*-------------------------Funciones auxiliares-------------------------*/
void * abrirArchivo(char * rutaArchivo, size_t * tamArc, FILE ** archivo) {
//Abro el archivo
	*archivo = fopen(rutaArchivo, "r");

	if (*archivo == NULL) {
		log_error(logInstancia, "%s: No existe el archivo", rutaArchivo);
		perror("Error al abrir el archivo");
		exit(EXIT_FAILURE);
	}

//Copio informacion del archivo
	struct stat statArch;

	stat(rutaArchivo, &statArch);

//Tamaño del archivo que voy a leer
	*tamArc = statArch.st_size;

//Leo el total del archivo y lo asigno al buffer
	int fd = fileno(*archivo);
	void * dataArchivo = mmap(0, *tamArc, PROT_READ, MAP_SHARED, fd, 0);

	return dataArchivo;
}

t_list * listarArchivosDeMismaCarpeta(char * ruta) {

	DIR *dir;

	struct dirent *ent;

	dir = opendir(ruta);

	if (dir == NULL)
		return NULL;

	t_list * listaArchivos = list_create();

	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] != '.') {
			char * archivo = string_new();
			string_append(&archivo, ruta);
			string_append(&archivo, "/");
			string_append(&archivo, (char*) ent->d_name);
			list_add(listaArchivos, archivo);
		}
	}

	closedir(dir);

	return listaArchivos;
}

int entradasNecesariaParaUnTamanio(int tamanio) {
	int entradasNecesaria = tamanio / tamanioEntrada;

	if (tamanio % tamanioEntrada != 0)
		entradasNecesaria++;

	return entradasNecesaria;
}

t_list * filtrarEntradasAtomicas(void) {
	bool entradaAtomica(t_tabla_entradas * registroTabla) {
		int entradasNecesarias = registroTabla->tamanio / tamanioEntrada;

		if (registroTabla->tamanio % tamanioEntrada != 0)
			entradasNecesarias++;

		return (entradasNecesarias == 1);
	}

	t_list * listaFiltrada = list_filter(tablaEntradas, (void*) entradaAtomica);

	bool ordenarMenorMayor(t_tabla_entradas * entrada,
			t_tabla_entradas * entradaMenor) {
		return entrada->indexComienzo < entradaMenor->indexComienzo;
	}

	list_sort(listaFiltrada, (void*) ordenarMenorMayor);

	return listaFiltrada;
}
