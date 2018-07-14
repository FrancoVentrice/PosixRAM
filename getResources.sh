# https://github.com/sisoputnfrba/tp-2018-1c-PosixRAM.git

echo "***********************************************************************"
echo "Descargando commons library"
cd $HOME
git clone https://leatex:utnfrbaso2018@github.com/sisoputnfrba/so-commons-library.git
echo "Instalando commons library"
cd so-commons-library
make
sudo make install
echo "."
echo "***********************************************************************"
echo "Descargando Parsi"
cd $HOME
git clone https://leatex:utnfrbaso2018@github.com/sisoputnfrba/parsi.git
echo "Instalando Parsi"
cd parsi
make
sudo make install
echo "."
echo "***********************************************************************"
echo "Descargando pruebas ESI"
cd $HOME
git clone https://leatex:utnfrbaso2018@github.com/sisoputnfrba/Pruebas-ESI.git

# readline debería estar instalada
# revisar /usr/include
# sudo apt-get install libreadline6 libreadline6-dev