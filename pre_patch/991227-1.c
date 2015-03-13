char* doit(int flag)
{
  return 1 + (flag ? "fwrong\n" : "fright\n");
}
int main()
{
  char *result = doit(0);
  if (*result == 'r' && result[1] == 'i')
    exit(0);
  abort();
}
