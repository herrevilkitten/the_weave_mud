

char *spaces( int number)
{
  char str[80], pt;
  int loop;

  if (number >= 80)
    number = 80;

  pt = str;

  for ( loop = 0; loop < number; loop++)
    *pt++ = ' ';

  *pt = '\0';
  return str_dup( str);
}
