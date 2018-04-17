#include "ipcprotocol.h"

/********************************************************************************************/
void crearHeader(HEADERIPC *cabecera, unsigned char ucPayloadDescriptor, unsigned int uiPayloadLength)
{
	generarID(cabecera->cpDescriptorID);
	cabecera->ucPayloadDescriptor = ucPayloadDescriptor;
	cabecera->uiPayloadLength = uiPayloadLength;

}

/********************************************************************************************/
void generarID(unsigned char* ucpID)
{
	int i,k;
	for (i=0; i<16;i++)
	{
		k= rand()%(2-0+1)+0;
		if (k==0) ucpID[i]=rand()%('z'-'a'+1)+'a';
		else if(k==1) ucpID[i]=rand()%('9'-'0'+1)+'0';
		else ucpID[i]=rand()%('Z'-'A'+1)+'A';
	}
}
