# UTN - FRBA
## Sistemas Operativos
## 1 C 2018

# T.P. Re Distinto
# Equipo PosixRAM

### Integrantes

| Nombre                    | Legajo   | E-mail                      | Curso  |
| ------------------------- |:--------:| --------------------------- | ------ |
| Franco Ventrice Goskarian | 146866-2 | goska2732@gmail.com         | K-3054 |
| Joel Akiyoshi             | 149868-0 | joelakiy.23@gmail.com       | K-3052 |
| Leandro M. Malsam         | 115350-0 | leandro.malsam@gmail.com    | K-3052 |
| Pablo Quinci              | 142025-2 | pablo.quinci@gmail.com      | K-3052 |

#### De baja

| Nombre                    | Legajo   | E-mail                      | Curso  |
| ------------------------- |:--------:| --------------------------- | ------ |
| Juan Paulo Vispo          | 120573-0 | juanpaulovispo@yahoo.com.ar | K-3052 |


### Despliegue automático

1- Descargar el repositorio: `git clone https://github.com/sisoputnfrba/tp-2018-1c-PosixRAM.git`

2- Asignar permisos de ejecución a los scripts bash: `chmod a+wrx *.sh`

3- Descargar los repositorios necesarios e instalar las dependencias: `./getResources.sh`

4- Compilar todo el TP: `source prepareEnvironment.sh`

5- En cada nueva consola exportar el path de librería: `source exportPath.sh`

### Ejecución de pruebas
Para cada prueba...

1- Ingresar al directorio que corresponda.

2- Preparar los archivos para las pruebas: `./prepararPrueba.sh`

3- Ingresar a cada componente y revisar los archivos de configuración.

### Limpieza

Para eliminar todos los directorios y dejar todo como estaba: `./cleanAll.sh`