# ejecutar con: source prepareEnvironment.sh
# para que la variable se mantenga en el contexto de la terminal

echo "***********************************************************************"
echo "Compilando Shared Library"
echo "***********************************************************************"
cd $HOME/tp-2018-1c-PosixRAM/shared/Default
make clean all
echo "***********************************************************************"
echo "Preparando variable LD_LIBRARY_PATH"
echo "***********************************************************************"
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$HOME/tp-2018-1c-PosixRAM/shared/Default
echo "$LD_LIBRARY_PATH"
echo "***********************************************************************"
echo "Compilando Instancia"
echo "***********************************************************************"
cd $HOME/tp-2018-1c-PosixRAM/Instancia/Default
make clean all
echo "***********************************************************************"
cd $HOME
