#include "serializar.h"

int serializar(void* buffer, const char* format, ...){
	va_list objs;
	int i = 0, buffIndex = 0, sizeObj;
	va_start(objs, format);
	void * pObj;

	while(format[i] != '\0'){
		if( format[i] == '%'){
			i++;
			switch( format[i] ){
				case 'h':{
					short obj = (short)va_arg( objs, int );
					sizeObj = sizeof(short);
					pObj = &obj;
					break;}
				case 'd':{
					int obj = (int)va_arg( objs, int );
					sizeObj = sizeof(int);
					pObj = &obj;
					break;}
				case 'c':{
					char obj = (char)va_arg(objs, int);
					sizeObj  = sizeof(char);
					pObj = &obj;
					break;}
				case 's':{
					char *obj = va_arg(objs, char*);
					sizeObj = strlen(obj)+1;
					pObj = obj;
					break;}
				default:
					va_end(objs);
					return -1;
			} /* FIN DE DETECCION DE CARACTER POST-'%' */
		} /* FIN DEL IF '%' */
		
		memcpy(buffer+buffIndex,pObj, sizeObj);
		buffIndex += sizeObj;
		i++;
	} /* FIN DEL WHILE RECORRE-CADENA */

	va_end(objs);

	return buffIndex;
}

int deserializar(void* buffer, const char* format, ...){
	va_list objs;
	int i = 0, buffIndex = 0, sizeObj;
	va_start(objs, format);
	void * obj;

	while(format[i] != '\0'){
		if( format[i] == '%'){
			i++;
			switch( format[i] ){
				case 'h':
					obj = (short *)va_arg( objs, int );
					sizeObj = sizeof(short);
					break;
				case 'd':
					obj = (int *)va_arg( objs, int );
					sizeObj = sizeof(int);
					break;
				case 'c':
					obj = (char *)va_arg(objs, int);
					sizeObj  = sizeof(char);
					break;
				case 's':
					obj = (char *)va_arg(objs, char*);
					sizeObj = strlen(buffer+buffIndex)+1;
					break;
				default:
					va_end(objs);
					return -1;
			} // FIN DE DETECCION DE CARACTER POST-'%'
		} // FIN DEL IF '%'
		memcpy(obj, buffer+buffIndex, sizeObj);
		buffIndex += sizeObj;
		i++;
	} // FIN DEL WHILE RECORRE-CADENA

	va_end(objs);

	return buffIndex;
}
