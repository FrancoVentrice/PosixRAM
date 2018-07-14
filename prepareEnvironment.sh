# ejecutar con: source prepareEnvironment.sh
# para que la variable se mantenga en el contexto de la terminal

echo "***********************************************************************"
echo "***********************************************************************"
echo "Compilando Shared Library"
echo "***********************************************************************"
cd $HOME/tp-2018-1c-PosixRAM/shared/Default
make clean all
echo "***********************************************************************"
echo "***********************************************************************"
echo "Preparando variable LD_LIBRARY_PATH"
echo "***********************************************************************"
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$HOME/tp-2018-1c-PosixRAM/shared/Default
echo "$LD_LIBRARY_PATH"
echo "***********************************************************************"
echo "***********************************************************************"
echo "Compilando ESI"
echo "***********************************************************************"
cd $HOME/tp-2018-1c-PosixRAM/ESI/Default
make clean all
echo "***********************************************************************"
echo "***********************************************************************"
echo "Compilando Instancia"
echo "***********************************************************************"
cd $HOME/tp-2018-1c-PosixRAM/Instancia/Default
make clean all
echo "***********************************************************************"
echo "***********************************************************************"
echo "Compilando Planificador"
echo "***********************************************************************"
cd $HOME/tp-2018-1c-PosixRAM/Planificador/Default
make clean all
echo "***********************************************************************"
echo "***********************************************************************"
echo "Compilando Coordinador"
echo "***********************************************************************"
cd $HOME/tp-2018-1c-PosixRAM/Coordinador/Default
make clean all
echo "***********************************************************************"
cd $HOME
chmod -R a+wrx $HOME/tp-2018-1c-PosixRAM/pruebas/PruebaDeadlock/prepararPrueba.sh
chmod -R a+wrx $HOME/tp-2018-1c-PosixRAM/pruebas/PruebaDistribucion/prepararPrueba.sh
chmod -R a+wrx $HOME/tp-2018-1c-PosixRAM/pruebas/PruebaEstres/prepararPrueba.sh
chmod -R a+wrx $HOME/tp-2018-1c-PosixRAM/pruebas/PruebaMinima/prepararPrueba.sh
chmod -R a+wrx $HOME/tp-2018-1c-PosixRAM/pruebas/PruebaPlanificacion/prepararPrueba.sh
chmod -R a+wrx $HOME/tp-2018-1c-PosixRAM/pruebas/PruebaReemplazoCompactacion/prepararPrueba.sh
