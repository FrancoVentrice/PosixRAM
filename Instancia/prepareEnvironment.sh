# ejecutar con: source prepareEnvironment.sh
# para que la variable se mantenga en el contexto de la terminal
echo "Eliminando logs"
rm -rf logs
rm -rf Debug/logs
echo "Compilando instancia"
cd Debug
make clean
make
echo "Preparando variable LD_LIBRARY_PATH"
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:~/workspace/tp-2018-1c-PosixRAM/shared/Debug
echo "$LD_LIBRARY_PATH"
