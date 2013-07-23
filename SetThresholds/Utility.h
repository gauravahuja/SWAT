inline bool isNumberInBetween(double Number, double LowNum , double HighNum)
{
	if((Number - LowNum >= 0) && (HighNum - Number >=0))
	{
		return true;
	}
	else
		return false;
}
inline bool isColourInBetween(unsigned char Number, unsigned char LowNum , unsigned char HighNum)
{
	if((Number - LowNum >= 0) && (HighNum - Number >=0))
	{
		return true;
	}
	else
		return false;
}