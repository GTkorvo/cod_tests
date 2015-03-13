extern void abort (void);

static char arg0[] = "arg0";
static char arg1[] = "arg1";

static void 
attr_rtx (char *varg0, char *varg1)
{
  if (varg0 != arg0)
    abort ();

  if (varg1 != arg1)
    abort ();

  return;
}

static char *
attr_string (str)
     char *str;
{
  return str;
}

static void 
attr_eq (name, value)
    char *name;
    char *value;
{
    attr_rtx (attr_string (name),
	      attr_string (value));
    return;
}

int main()
{
  attr_eq (arg0, arg1);
  exit (0);
}

