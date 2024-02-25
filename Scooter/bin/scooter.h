#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef checker
	#ifndef stdlib
		#include <stdlib.h>
	#endif
	extern void _exit(register int);	// corrisponde all'etichetta 
						// _exit: mov rax, 60
						// 	  mov rdi, 1 ; 1 corrisponde a EXIT_FAILURE per comunicare l'uscita con successo si usa 0
						// 	  syscall

	#define checker(x, msg) 		\
		if(x){				\
			printf("%s\n", msg); 	\
			_exit(EXIT_FAILURE);	\
		}
#else
	#error, "macro checker already defined"
#endif


#ifndef buffer_x_all
	// provo a ricreare anche la proprieta di classe chiamata all[] di python, che colleziona tutti gli oggetti creati, es:
	// class Scooter():
	// 	all = []
	// 	def __init__(self, x):
	// 		self.x = x
	// 		Scooter.all.append(self)
	//
	// non c'entra con l'esercizio, la voglio solo supporre un'implementazione della all[]
	// sarebbe un vettore di struct scooter
	// se dealloco una struct da main si dovrebbe liberare lo spazio precedentemente occupato dalla struct nel vettore all
	#define buffer_x_all 5000
	struct scooter* all[buffer_x_all];
#else
	#error, "MACRO buffer_x_all already defined"
#endif


/*
 *
 *	nome: 		Federico Bastianello
 *	classe: 	4AA
 *	data: 		24 / 02 / 2025
 *	
 *	consegna: 	1) Implementare in C la classe Scooter, inserendo i controlli nel costruttore e fornendo uno
 *			o piu metodi per trasformare un oggetto in una stringa. Commentare opportunamente il codice.
 *			
 *			filosofia della struttura sulla classe Scooter:
 *			- Uno scooter possiede un serbatoio con una certa capacita (in litri), una certa quantita	
 *			  di carburante nel serbatoio, una determinata resa del carburante (misurata in km/l cioe'
 *			  chilometri / litro) ed un contachilometri per registrare i kmPercorsi.
 *			
 *			- il metodo rifornimeto() serve per aggiungere carburante nel serbatoio
 *			
 *			- il metodo avanza() data la distanza da percorrere, riduce la quantita di carburante nel
 *			  serbatoio a seconda della resa e aggiorna il valore del contachilometri
 *			
 *			- il metodo reset_km() azzera il contachilometri
 *			
 *			- la classe e' in grado di tenere traccia di quanti scooter sono stati creati.
 *			
 *			- Ciascun scooter nasce con un codice id. univoco
 *			
 *			- le variabili di istanza devono essere protette, in particolare capacita e resa sono visibili
 *			  ma non modificabili; mentre quantita di carburante e km percorsi sono visibile e possono
 *			  essere modificati solo tramite l'uso dei metodi avanza(), rifornimento() e reset().
 *			
 *			- Il numero di scooter non deve essere modificabile, ma solo leggibile
 *			
 *			- lo stesso vale per codice identificante di ciascun scooter  
 * 
*/


// tiene traccia di quanti scooter vengono creati
static unsigned int KEY_ID = 0;


struct scooter{
	//proprieta classe
	unsigned int
		id;
	float 
		capacita, 
		quantita,
		resa,
		contachilometri;

	// puntatori alle funzioni della classe
	void 
		(*rifornimento) (struct scooter*, float),
	 	(*avanza) 	(struct scooter*, float),
		(*reset)	(struct scooter*),
	 	(*__del__) 	(struct scooter*);

	char* 
		(*__str__) 	(struct scooter*);
};


struct scooter*		Scooter		(float, float, float, float);

// le funzioni static sono accessibili solo tramite i ptr a funzione descritti nella struct scooter e sono visibili solo a questo modulo
// il token inline e' per il compilatore, che se puo migliorera le prestazioni del programma 
static inline void 	__rifornimento	(struct scooter*, float);
static inline void 	__avanza	(struct scooter*, float);
static inline void	__reset		(struct scooter*);
static inline char* 	__str		(struct scooter*);
static inline void 	__del		(struct scooter*);


struct scooter* Scooter(float capacita, float quantita, float resa, float contachilometri){
	KEY_ID++;

	struct scooter* __init__ = (struct scooter*) malloc(sizeof(struct scooter));
	checker(__init__ == NULL, "errore nella creazione di init");

	// assegnazione proprieta
	// come si potrebbero rendere private ? 
	// con static non si puo perche gia all'interno di questa funzione non sono piu' modificabili

	__init__ -> id 			= KEY_ID;
	__init__ -> capacita 		= (capacita > 1.0)				? capacita 		: 5.0;
	__init__ -> quantita 		= ((quantita > 0.0) && (quantita < capacita))	? quantita 		: capacita - 1.0;
	__init__ -> resa 		= (resa > 0.0)					? resa 			: 1.0;
	__init__ -> contachilometri 	= (contachilometri >= 0)			? contachilometri 	: 0.0;


	// assegnazione costrutti ai puntatori
	__init__ -> rifornimento	= __rifornimento;
	__init__ -> avanza		= __avanza;
	__init__ -> reset		= __reset;
	__init__ -> __str__		= __str;
	__init__ -> __del__		= __del;

	
	/*	algoritmo per assegnare al vettore all le struct 	*/
	unsigned int i = 0;
	while(all[i] != NULL){ 	// scorro la lista finche non trovo uno spazio vuoto
		if(i == buffer_x_all){
			printf("\nsono stati creati %i oggetti, stai sforando il buffer dell'array di struct all\n", buffer_x_all);
			goto exceptOverflow;
		}
		i++;
	}
	all[i] = __init__; 	// memorizzo l'oggetto nel vettore all se ce spazio

	exceptOverflow:
		return __init__;
}


static inline void __rifornimento(struct scooter* oggetto, float carburante){
	// funzione che serve per aggiungere carburante al serbatoio

	checker(carburante <= 0.0, "errore in rifornimento(), carburante' e negativo");

	// se quantita presente nel serbatoio + il carburante che aggungo e' maggiore della capacita massima
	oggetto->quantita = (oggetto->quantita + carburante > oggetto->capacita)? oggetto->capacita : oggetto->quantita + carburante;
}


static inline void __avanza(struct scooter* oggetto, float distanza){
	// funzione che data la distanza da percorrere riduce la quantita di carburante in base alla resa e aggiorna il contachilometri

	// verifico che il parametro distanza e' maggiore di 0 e che ci sia carburante nell'oggeto
	checker((distanza < 0.0) || (oggetto -> quantita > 0.0), "errore nel metodo avanza()");

	float km_possibili = oggetto->quantita * oggetto->resa;

	if(distanza <= km_possibili){
		oggetto -> quantita 		-= distanza / oggetto->resa;
		oggetto -> contachilometri 	+= distanza;
	}
}


static inline void __reset(struct scooter* oggetto){
	// Metodo usato per resettare il contachilometri dello scooter
	oggetto -> contachilometri = 0.0;
}


static inline char* __str(struct scooter* oggetto){
	#ifndef BUFFER_SIZE
		#define BUFFER_SIZE 200

		#define property \
			oggetto -> capacita, oggetto -> quantita, oggetto -> resa, oggetto -> contachilometri, oggetto -> id, KEY_ID
	#else
		free(oggetto);
		#error, "macro BUFFER_SIZE already defined"
	#endif

	static char stringa[BUFFER_SIZE];

	sprintf(stringa, "capacita: %.2f, quantita carburante: %.2f, resa: %.2f, kmPercorsi: %.2f, id oggetto: %i, numero oggetti creati: %i", property);
	return stringa;
}


static inline void __del(struct scooter* oggetto){
	free(oggetto);		// credo che venga libera la memoria anche per il vettore all[]
	printf("oggetto eliminato\n");
}

