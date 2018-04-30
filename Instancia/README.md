# Re Distinto
## Instancia

### Preparar ambiente de ejecución
Se deberá setear la variable `LD_LIBRARY_PATH` en cada contexto o terminal que se desee ejecutar la instancia.
Para ello existe un script bash que se invoca de la siguiente manera:

```
source prepareEnvironment.sh
```

El script:

1. Elimina las carpetas de logs existentes.
2. Ejecuta el `make` para la instancia.
3. Setea la variable `LD_LIBRARY_PATH`.

### Archivo de configuración
Archivo de ejemplo:
```
IP_COORDINADOR=127.0.0.1
PUERTO_COORDINADOR=8000
ALGORITMO_REEMPLAZO=CIRC
PUNTO_MONTAJE=/home/utnso/Escritorio/mount
NOMBRE_INSTANCIA=ROBOCOP
INTERVALO_DUMP=180
```
Ver: [Configuración de Instancia](https://sisoputnfrba.gitbook.io/re-distinto/instancia#configuracion)

### Ejecución

Se puede ejecutar sin parámetros: `./Instancia`

En este caso busca el archivo de configuración por defecto `Instancia.conf` para poder levantarse.

También se puede ejecutar pasando el archivo de configuración como parámetro: `./Instancia InstUno.conf`

Esto permite ejecutar distintas instancias con distintos archivos de configuración.

## (C) PosixRAM 2018