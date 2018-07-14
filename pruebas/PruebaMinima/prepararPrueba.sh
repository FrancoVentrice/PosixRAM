cp -f Coordinador.conf $HOME/tp-2018-1c-PosixRAM/Coordinador/Default/
cp -f Planificador.conf $HOME/tp-2018-1c-PosixRAM/Planificador/Default/
cp ESI.conf $HOME/tp-2018-1c-PosixRAM/ESI/Default/
rm -f $HOME/tp-2018-1c-PosixRAM/ESI/Default/ESI_*
cp -f ESI_* $HOME/tp-2018-1c-PosixRAM/ESI/Default/
cp -f Inst1.conf $HOME/tp-2018-1c-PosixRAM/Instancia/Default/
cp -f Inst2.conf $HOME/tp-2018-1c-PosixRAM/Instancia/Default/
cp -f Inst3.conf $HOME/tp-2018-1c-PosixRAM/Instancia/Default/
rm -f -r -d /home/utnso/inst1/
rm -f -r -d /home/utnso/inst2/
rm -f -r -d /home/utnso/inst3/