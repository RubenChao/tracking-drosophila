/*
 * Errores.hpp
 *
 *  Created on: 22/09/2011
 *      Author: chao
 */

#ifndef ERRORES_HPP_
#define ERRORES_HPP_

//!brief Contiene los tipos de errores.
/*!
 *
 * \return En caso de error se visualiza un mensage por pantalla y se para la ejecució del programa.
 *
 * Ejemplo:
 *
 * \verbatim

		case 1:
			fprintf( stderr, "ERROR: No se puede abrir el video. Esto puede suceder"
					"por:\n - El formato de video no está soportado.\n - Los códecs"
					"no están correctamente instalados.\n - El video está corrupto.\n");

		case 2:
			fprintf( stderr, "ERROR: frame is null...Vídeo corrupto.\n" ); //getchar();
			DestroyWindows( );
			break;

		case 3:

			fprintf( stderr, "ERROR: No se ha encontrado el plato \n" );
			fprintf( stderr, "Puede ser debido a:\n "
					"- Tipo de video cargado incorrecto "
					"- Iluminación deficiente \n"
					"- Iluminación oblicua que genera sombras pronunciadas\n"
					"- Plato  fuera del área de visualización\n" );
		case 4:

				fprintf( stderr, "ERROR: Memoria insuficiente\n" );

		case 5:
				fprintf( stderr, "ERROR: División por 0 \n" );

		case 6:

				fprintf( stderr, "ERROR: Fallo al guardar datos 0 \n" );

		case 7:
			fprintf( stderr, "ERROR: Fallo al liberar buffer 0 \n" );
	\endverbatim
 */
	void error(int);

#endif /* ERRORES_HPP_ */
