typedef unsigned char byte_t;

int getSetFrmCmd(byte_t cmd)
{
	return (cmd>>6)&3;
}

int getOprFrmCmd(byte_t cmd)
{
	return (cmd>>3)&7;
}

int getValFrmCmd(byte_t cmd)
{
	return (cmd>>2)&1;
}
