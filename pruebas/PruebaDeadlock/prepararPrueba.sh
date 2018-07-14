cp -f Coordinador.conf $HOME/tp-2018-1c-PosixRAM/Coordinador/
cp -f Planificador.conf $HOME/tp-2018-1c-PosixRAM/Planificador/
cp ESI.conf $HOME/tp-2018-1c-PosixRAM/ESI/
rm -f $HOME/tp-2018-1c-PosixRAM/ESI/ESI_*
cp -f ESI_* $HOME/tp-2018-1c-PosixRAM/ESI/
cp Inst1.conf $HOME/tp-2018-1c-PosixRAM/Instancia/
cp Inst2.conf $HOME/tp-2018-1c-PosixRAM/Instancia/
rm -f -r -d /home/utnso/inst1/
rm -f -r -d /home/utnso/inst2/