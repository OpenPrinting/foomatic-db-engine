/*
 *   Foomatic Combo XML
 *   ------------------
 *
 *   Compute a Foomatic printer/driver combo by a pure C program doing
 *   a simplified sequential-reading XML-parsing. Perl libraries which
 *   make Perl data structures out of the XML files are very slow and
 *   memory-consuming.
 *
 *   Copyright 2001-2007 by Till Kamppeter
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of the
 *   License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *   02111-1307  USA
 *
 */

/*
 * Include necessary headers (standard libraries)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

/*
 * Data structures for the printer/driver combo by printer list for the
 * overview ("-O" option)
 */

typedef struct { /* structure for a driver entry (linear list) */
  char                  name[128]; /* Name of driver */
  char                  *functionality; /* Exceptions in driver
                                      functionality profile for this
				      printer */
  struct driverlist_t   *next;     /* pointer to next driver */
} driverlist_t;

typedef struct { /* structure for a printer entry (linear list) */
  char                  id[128];   /* ID of printer */
  driverlist_t          *drivers;  /* pointer to the list of the drivers
				      with which this printer works */
  struct printerlist_t  *next;     /* pointer to next printer */
} printerlist_t;

typedef struct { /* structure for a ready-made PPD entry (linear list) */
  char                  driver[128]; /* ID of driver */
  char                  ppd[1024];   /* ID of PPD URL */
  struct ppdlist_t  *next;           /* pointer to next PPD */
} ppdlist_t;

typedef struct { /* structure for printer ID translations (linear list) */
  char                  *oldid,    /* old ID of printer */
                        *newid;    /* current ID of printer */
  struct idlist_t       *next;     /* pointer to next entry */
} idlist_t;

/*
 * function to load a file into the memory
 */

char  /* O - pointer to the file in memory */
*loadfile(const char *filename) { /* I - file name */
  
  const int blocksize = 1024 * 1024;
  FILE *inputfile;          /* file to be read currently */
  char buffer[blocksize + 1];/* data block currently read */
  char *data = NULL;        /* the read data */
  int size = 1;             /* size of the data in memory */
  int bytesread;            /* bytes actually read */

  /* Open the file */

  inputfile = fopen(filename, "r");
  if (inputfile == NULL) {
    fprintf(stderr, "Cannot read file %s!\n", filename);
    return NULL;
  }

  /* Read the whole file into the memory */

  data = (char *)malloc(size);
  data[0] = '\0';
  while((bytesread = fread(buffer,1,blocksize,inputfile))) {
    data = (char *)realloc(data, size + bytesread); 
    buffer[bytesread] = '\0';
    strcat(data, buffer);
    size += bytesread;
  }
  fclose(inputfile);
  /* Make space for additional data */
  data = (char *)realloc(data, size + 4096); 
  if (strcmp(data, "")) return(data); else { free((void *)data); return(NULL); }
}

/*
 * function to load the printer ID translation table
 */

idlist_t /* O - pointer to the printer ID translation table */
*loadidlist(const char *filename) { /* I - file name */
  char          *idlistbuffer = NULL; /* ID list in memory */
  char          *scan; /* pointer for scanning through the list */
  char          *oldid = NULL,
                *newid = NULL; /* pointers to IDs in the current line */
  int           inoldid = 0, /* Are we reading and old or a new ID */
                innewid = 0; /* currently */
  idlist_t      *idlist = NULL, /* Pointer to ID list */
                *currentitem = NULL, /* Pointer to current ID list item */
                *newitem; /* Pointer to newly created ID list item */

  idlistbuffer = loadfile(filename);
  if (!idlistbuffer) return NULL;
  for (scan = idlistbuffer; *scan != '\0'; scan++) {
    switch(*scan) {
    case '\r':
    case '\n': /* line break */
      /* line end */
      if (inoldid) {
	/* Line with only one word, ignore it */
	inoldid = 0;
	oldid = NULL;
	break;
      }
    case '\t': /* tab */
    case ' ': /* space */
      /* white space */
      if (inoldid) {
	/* old ID completed */
	inoldid = 0;
	*scan = '\0';
      }
      if (innewid) {
	/* new ID completed */
	innewid = 0;
	*scan = '\0';
	if (oldid && newid && (*oldid != '#')) {
	  /* found a pair of old ID and new ID, add it to the translation
	     list */
	  newitem = 
	    (idlist_t *)malloc(sizeof(idlist_t));
	  newitem->oldid = oldid;
	  newitem->newid = newid;
	  newitem->next = NULL;
	  if (currentitem) {
	    currentitem->next = (struct idlist_t *)newitem;
	  } else {
	    idlist = newitem;
	  }
	  currentitem = newitem;
	}
	oldid = NULL;
	newid = NULL;
      }
      break;
    default:
      /* all other characters, so no whitespace now */
      if (!(inoldid || innewid)) {
	/* last character was whitespace */
	if (oldid) {
	  /* Beginning of second word in the line */
	  innewid = 1;
	  newid = scan;
	} else {
	  /* Beginning of first word in the line */
	  inoldid = 1;
	  oldid = scan;
	}
      }
    }
  }
  return idlist;
}

/*
 * function to translate an old printer ID into a new one
 */

char  /* O - new ID */
*translateid(const char *oldid, /* I - Old ID */
	     idlist_t *idlist) { /* I - ID translation table */
  idlist_t      *currentitem = idlist; /* Pointer to current ID list item */

  while(currentitem) {
    if (strcmp(oldid, currentitem->oldid) == 0) {
      return currentitem->newid;
    }
    currentitem = (idlist_t *)(currentitem->next);
  }
  return (char *)oldid;
}

/*
 * function to parse an XML file and do a task on it
 */

int /* O - Is the requested printer driver combo already confirmed by the
           <drivers> section in the printer XML file (operation = 0 only) */
parse(char **data, /* I/O - Data to process */
      const char *pid,   /* I - Foomatic printer ID */
      const char *driver,/* I - driver name */
      const char *filename, /* I - file name for error messages */
      printerlist_t **printerlist, /* I/O printer list for overview */
      int operation,     /* I - Operation type: 0: printer, 1: driver, 
			        2: option 3: overview driver 4: overview
			        printer */
      const char **defaultsettings, /* I - Default option settings given on
				       the command line */
      int num_defaultsettings, /* I - Number of default option settings */
      int *nopjl,        /* I/O - 0: driver allows PJL options, 1: driver
			    does not allow PJL options (has "<nnpjl />"
			    flag in "<execution>" section) */
      idlist_t *idlist,  /* I - ID translation table */
      int debug) {       /* I - Debug flag: If set, debugging output is
			        produced */

  char          *trpid = NULL; /* current printer ID translated according
				  to translation table */
  int           datalength = 0;  /* length of the XML file */
  int           linecount = 1;   /* Count the lines for error messages */
  int           nestinglevel = 0;/* How many XML environments are nested
				    at the point where we are */
  int           inxmlheader = 1;
  int           intag = 0;
  int           incomment = 0;
  int           tagnamefound = 0;
  int           intagword = 0;
  int           inquotes = 0;
  int           insinglequotes = 0;
  int           indoublequotes = 0;
  int           tagtype = 0; /*1: opening, -1: closing, 0: opening & closing*/
  int           inprinter = 0;
  int           inmake = 0;
  int           inmodel = 0;
  int           inautodetect = 0;
  int           indriver = 0;
  int           indrivers = 0;
  int           inexecution = 0;
  int           inprototype = 0;
  int           innopjl = 0;
  int           inprinters = 0;
  int           inid = 0;
  int           inppd = 0;
  int           inlang = 0;
  int           inpostscript = 0;
  int           inoption = 0;
  int           inargshortname = 0;
  int           inargexecution = 0;
  int           inargpjl = 0;
  int           inevshortname = 0;
  int           inen = 0;
  int           inargmax = 0;
  int           inargmin = 0;
  int           inenumval = 0;
  int           inconstraints = 0;
  int           inconstraint = 0;
  int           inargdefault = 0;
  int           infunctionality = 0;
  int           inunverified = 0;
  int           indfunctionality = 0;
  int           incomments = 0;
  int           printertobesaved = 0;
  int           printerentryfound = 0;
  int           constrainttoberemoved = 0;
  int           enumvaltoberemoved = 0;
  int           optionqualified = 0;
  int           enumvalqualified = 1;
  int           numenumvals = 1; /* Number of enumeration values,
				    disqualifies option when 0 at the end of
				    the file. It is set to zero in the 
				    beginning of an enum option, for boolean
				    or numerical options it stays 1 to not
				    disqualify the option */
  int           optiontype = 0; /* 0: enum, 1: bool, 2: int, 3: float */
  int           printerscore = 0;
  int           driverscore = 0;
  int           printerhiscore = 0;
  int           driverhiscore = 0;
  int           defaultlinelength = 0;
  char          currtagname[256];
  char          currtagparam[256];
  char          currtagbody[65536];
  char          optiondefault[256];
  int           userdefault = 0;
  int           userdefaultfound = 0;
  char          userdefaultvalue[256];
  char          userdefaultid[256];
  char          currevid[256];
  double        maxnumvalue = 0;
  double        minnumvalue = 0;
  int           csense = 0;
  char          cprinter[256];
  char          cmake[256];
  char          cmodel[256];
  char          cdriver[256];
  char          cid[256];
  char          cppd[1024];
  char          cfunctionality[256];
  int           cunverified = 0;
  char          cautodetectentry[4096];
  char          cargdefault[256];
  char          argdefault[256];
  char          defaultline[256];
  char          printerentry[1024*1024];
  char          dfunctionalityentry[10240];
  const char    *scan;               /* pointer for scanning through the file*/
  const char    *lasttag = NULL;     /* Start of last XML tag */
  const char    *lasttagend = NULL;  /* End of last XML tag */
  const char    *tagwordstart = NULL;/* Beginning of tagname */
  char    *lastprinters = NULL;      /* Start of last <printers> tag */
  char    *lastprinter = NULL;       /* Start of last <printer> tag */
  char    *lastenumval = NULL;       /* Start of last <enum_val> tag */
  char    *lastconstraints = NULL;   /* Start of last <constraints> tag */
  char    *lastoption = NULL;        /* Start of last <option> tag */
  char    *lastautodetect = NULL;    /* Start of last <autodetect> tag */
  char    *lastdfunctionality = NULL;/* Start of last <functionality> tag */
  char    *lastcomments = NULL;      /* Start of last <comments> tag */
  char    *lastprototype = NULL;     /* Start of last <prototype> tag */
  static char   make[256];           /* Printer make/model read from printer */
  static char   model[256];          /* XML file needed by constraints in */
                                     /* option XML files */
  int           comboconfirmed = 0;
  int           driverhasproto = 0;
  int           exceptionfound = 0;

  char          *s;
  int           l;
  int           j;
  int           k;
  printerlist_t *plistpointer;   /* pointers to navigate through the printer */
  driverlist_t  *dlistpointer;   /* list for the overview */
  ppdlist_t     *ppdlistpointer;
  printerlist_t *plistpreventry;
  driverlist_t  *dlistpreventry;
  driverlist_t  *dlistnextentry;
  ppdlist_t     *ppdlistpreventry;
  ppdlist_t     *ppdlist = NULL;

  /* Translate printer ID */
  if ((pid) && (operation < 3)) trpid = translateid(pid, idlist);

  j = 0;
  datalength = strlen(*data); /* Compute the length of the file once,
			         this is very time-intensive */
  if (operation == 1) *nopjl = 0; /* When we parse a driver, put the nopjl
				     flag to zero, the driver can switch it
				     to 1 when it contains "<nopjl />" */
  for (scan = *data; *scan != '\0'; scan++) {
    switch(*scan) {
    case '<': /* open angle bracket */
      if (!inquotes) {
	if (intag) {
	  if (!incomment && !inxmlheader) {
	    /* Unless a tag is a comment, angle brackets cannot appear inside
	       the tag. */
	    fprintf(stderr, "XML error: Nested angle brackets in %s, line %d!\n",
		    filename, linecount);
	    exit(1);
	  }
          /* In a comment angle brackets are treated as a part of a word */
	  /*if (!intagword) {
	    intagword = 1;
	    tagwordstart = scan;
	    }*/
	} else {
	  intag = 1;
	  if (scan + 3 < *data + datalength) {
	    if ((*(scan + 1) == '!') && (*(scan + 2) == '-') &&
		(*(scan + 3) == '-')) {
	      incomment = 1;
	      tagtype = 0;
	      if (debug) fprintf(stderr, "    Start of a comment\n");
	    }
	  }
	  if (!incomment) {
	    tagnamefound = 0;
	    tagtype = 1;
	    lasttag = scan;
	  }
	}
      }
      break;
    case '\n': /* line break */
      linecount ++;
    case '/': /* tag closing */
    case '>': /* close angle bracket */
    case ' ': /* white space */
    case '\t':
    case '\r': /* Carriage return of dossy XML file */
      /* word delimiters */
      if (!inquotes) {
	if (intag) {
	  if (!incomment) {
	    if (intagword) { /* a word in the XML tag finished */
	      intagword = 0;
	      if (!tagnamefound) { /* 1st word = tag name */
		tagnamefound = 1;
		memmove(currtagname, tagwordstart, scan - tagwordstart);
		currtagname[scan - tagwordstart] = '\0';
		if (debug) fprintf(stderr, "    Tag Name: '%s'\n",
				   currtagname);
		if (operation == 0) { /* Printer XML file */
		  if (strcmp(currtagname, "make") == 0) {
		    inmake = nestinglevel + 1;
		  } else if (strcmp(currtagname, "model") == 0) {
		    inmodel = nestinglevel + 1;
		  } else if (strcmp(currtagname, "autodetect") == 0) {
		    inautodetect = nestinglevel + 1;
		  } else if (strcmp(currtagname, "driver") == 0) {
		    indriver = nestinglevel + 1;
		    if (indrivers) {
		      if (tagtype == 1) {
			if (debug)
			  fprintf(stderr, 
				  "    Resetting Driver.\n");
			cid[0] = '\0';
		      }
		    }
		  } else if (strcmp(currtagname, "drivers") == 0) {
		    indrivers = nestinglevel + 1;
		  } else if (strcmp(currtagname, "id") == 0) {
		    inid = nestinglevel + 1;
		  } else if (strcmp(currtagname, "printer") == 0) {
		    inprinter = nestinglevel + 1;
		    if (tagtype == 1) {
		      /* XML body of the file is starting here */
		      inxmlheader = 0;
		      nestinglevel = 1;
		      /* Remove the whole header of the XML file */
		      if (debug) 
			fprintf(stderr,
				"    Removing XML file header\n");
		      memmove(*data, lasttag, 
			      *data + datalength + 1 - lasttag);
		      datalength -= lasttag - *data;
		      scan -= lasttag - *data;
		      tagwordstart -= lasttag - *data;
		      lasttag = *data;
		      lasttagend = NULL;
		    }
		  } else if (strcmp(currtagname, "lang") == 0) {
		    inlang = nestinglevel + 1;
		  } else if (strcmp(currtagname, "postscript") == 0) {
		    inpostscript = nestinglevel + 1;
		    if (inlang) {
		      if (tagtype == 1) {
			if (debug)
			  fprintf(stderr, 
				  "    Resetting Driver/PPD.\n");
			cid[0] = '\0';
			cppd[0] = '\0';
		      }
		    }
		  } else if (strcmp(currtagname, "ppd") == 0) {
		    inppd = nestinglevel + 1;
		  }
		} else if (operation == 1) { /* Driver XML file */
		  if (strcmp(currtagname, "printer") == 0) {
		    inprinter = nestinglevel + 1;
		    if (tagtype == 1) lastprinter = (char*)lasttag;
		  } else if (strcmp(currtagname, "execution") == 0) {
		    inexecution = nestinglevel + 1;
		  } else if (strcmp(currtagname, "nopjl") == 0) {
		    innopjl = nestinglevel + 1;
		    if (inexecution) {
		      *nopjl = 1;
		      if (debug)
			fprintf
			  (stderr,
			   "      <nopjl /> found, driver does not allow PJL options!\n");
		    }
		  } else if (strcmp(currtagname, "id") == 0) {
		    inid = nestinglevel + 1;
		  } else if (strcmp(currtagname, "printers") == 0) {
		    inprinters = nestinglevel + 1;
		    if (tagtype == 1) {
		      /* Mark up to the end of the tag before, so that there do
			 not remain empty lines or other whitespace after
			 deleting this constraint */
		      lastprinters = (char*)lasttagend + 1;
		      printerentry[0] = '\0';
		    }
		  } else if (strcmp(currtagname, "driver") == 0) {
		    indriver = nestinglevel + 1;
		    if (tagtype == 1) {
		      /* XML body of the file is starting here */
		      inxmlheader = 0;
		      nestinglevel = 1;
		      /* Remove the whole header of the XML file */
		      if (debug) 
			fprintf(stderr,
				"    Removing XML file header\n");
		      memmove(*data, lasttag, 
			      *data + datalength + 1 - lasttag);
		      datalength -= lasttag - *data;
		      scan -= lasttag - *data;
		      tagwordstart -= lasttag - *data;
		      lasttag = *data;
		      lasttagend = NULL;
		    }
		  } 
		} else if (operation == 2) { /* Option XML file */
		  if (strcmp(currtagname, "make") == 0) {
		    inmake = nestinglevel + 1;
		  } else if (strcmp(currtagname, "model") == 0) {
		    inmodel = nestinglevel + 1;
		  } else if (strcmp(currtagname, "driver") == 0) {
		    indriver = nestinglevel + 1;
		  } else if (strcmp(currtagname, "printer") == 0) {
		    inprinter = nestinglevel + 1;
		  } else if (strcmp(currtagname, "arg_defval") == 0) {
		    inargdefault = nestinglevel + 1;
		  } else if (strcmp(currtagname, "arg_shortname") == 0) {
		    inargshortname = nestinglevel + 1;
		  } else if (strcmp(currtagname, "arg_execution") == 0) {
		    inargexecution = nestinglevel + 1;
		  } else if (strcmp(currtagname, "arg_pjl") == 0) {
		    inargpjl = nestinglevel + 1;
		    if (inargexecution) {
		      /* We have a PJL option ... */
		      if (*nopjl) {
			/* ... and the driver does not allow it. 
			   So skip this option. */
			free((void *)(*data));
			*data = NULL;
			if (debug)
			  fprintf
			    (stderr,
			     "      Driver does not allow PJL options and this is a PJL option -->\n    Option does not apply!\n");
			return;
		      }
		    }
		  } else if (strcmp(currtagname, "ev_shortname") == 0) {
		    inevshortname = nestinglevel + 1;
		  } else if (strcmp(currtagname, "en") == 0) {
		    inen = nestinglevel + 1;
		  } else if (strcmp(currtagname, "arg_max") == 0) {
		    inargmax = nestinglevel + 1;
		  } else if (strcmp(currtagname, "arg_min") == 0) {
		    inargmin = nestinglevel + 1;
		  } else if (strcmp(currtagname, "constraints") == 0) {
		    inconstraints = nestinglevel + 1;
		    if (tagtype == 1) {
		      /* Reset high scores */
		      printerhiscore = 0;
		      driverhiscore = 0;
		      /* Mark up to the end of the tag before, so that there do
			 not remain empty lines or other whitespace after
			 deleting this constraint */
		      lastconstraints = (char*)lasttagend + 1;
		    }
		  } else if (strcmp(currtagname, "constraint") == 0) {
		    inconstraint = nestinglevel + 1;
		    if (tagtype == 1) {
		      /* Delete the fields of the old constraint */
		      cprinter[0] = '\0';
		      cmake[0] = '\0';
		      cmodel[0] = '\0';
		      cdriver[0] = '\0';
		      cargdefault[0] = '\0';
		      csense = 0;
		    }
		  } else if (strcmp(currtagname, "enum_val") == 0) {
		    inenumval = nestinglevel + 1;
		    if (tagtype == 1) {
		      /* New enum value, enum values are qualified by default
			 and can be disqualified by constraints */
		      enumvalqualified = 1;
		      enumvaltoberemoved = 0;
		      /* Mark up to the end of the tag before, so that there do
			 not remain empty lines or other whitespace after
			 deleting this constraint */
		      lastenumval = (char*)lasttagend + 1;
		    }
		  } else if (strcmp(currtagname, "option") == 0) {
		    inoption = nestinglevel + 1;
		    /* Mark up to the end of the tag before, to insert the
		       definition of the default option setting */
		    if (tagtype == -1) lastoption = (char*)lasttagend + 1;
		    if (tagtype == 1) {
		      /* XML body of the file is starting here */
		      inxmlheader = 0;
		      nestinglevel = 1;
		      argdefault[0] = '\0';
		      /* Remove the whole header of the XML file */
		      if (debug) 
			fprintf(stderr,
				"    Removing XML file header\n");
		      memmove(*data, lasttag, 
			      *data + datalength + 1 - lasttag);
		      datalength -= lasttag - *data;
		      scan -= lasttag - *data;
		      tagwordstart -= lasttag - *data;
		      lasttag = *data;
		      lasttagend = NULL;
		    }
		  }
		} else if (operation == 3) { /* Driver XML file (Overview) */
		  if (strcmp(currtagname, "printer") == 0) {
		    inprinter = nestinglevel + 1;
		    if (tagtype == 1) {
		      cprinter[0] = '\0';
		      dfunctionalityentry[0] = '\0';
		    }
		  } else if (strcmp(currtagname, "id") == 0) {
		    inid = nestinglevel + 1;
		  } else if (strcmp(currtagname, "functionality") == 0) {
		    indfunctionality = nestinglevel + 1;
		    if (tagtype == 1) lastdfunctionality = (char*)lasttag;
		  } else if (strcmp(currtagname, "execution") == 0) {
		    inexecution = nestinglevel + 1;
		  } else if (strcmp(currtagname, "prototype") == 0) {
		    inprototype = nestinglevel + 1;
		    if (tagtype == 1) lastprototype = (char*)lasttagend + 1;
		  } else if (strcmp(currtagname, "printers") == 0) {
		    inprinters = nestinglevel + 1;
		    if (tagtype == 1) lastprinters = (char*)lasttagend + 1;
		  } else if (strcmp(currtagname, "comments") == 0) {
		    incomments = nestinglevel + 1;
		    if (tagtype == 1) lastcomments = (char*)lasttagend + 1;
		  } else if (strcmp(currtagname, "driver") == 0) {
		    indriver = nestinglevel + 1;
		    if (tagtype == 1) {
		      /* XML body of the file is starting here */
		      inxmlheader = 0;
		      nestinglevel = 1;
		      /* Remove the whole header of the XML file */
		      if (debug) 
			fprintf(stderr,
				"    Removing XML file header\n");
		      memmove(*data, lasttag, 
			      *data + datalength + 1 - lasttag);
		      datalength -= lasttag - *data;
		      scan -= lasttag - *data;
		      tagwordstart -= lasttag - *data;
		      lasttag = *data;
		      lasttagend = NULL;
		    }
		  } 
		} else if (operation == 4) { /* Printer XML file (Overview)*/
		  if (debug)
		    fprintf(stderr,
			    "     Printer XML (Overview): Tag name: %s\n",
			    currtagname);
		  if (strcmp(currtagname, "make") == 0) {
		    inmake = nestinglevel + 1;
		  } else if (strcmp(currtagname, "model") == 0) {
		    inmodel = nestinglevel + 1;
		  } else if (strcmp(currtagname, "functionality") == 0) {
		    infunctionality = nestinglevel + 1;
		  } else if (strcmp(currtagname, "unverified") == 0) {
		    inunverified = nestinglevel + 1;
		    cunverified = 1;
		  } else if (strcmp(currtagname, "driver") == 0) {
		    indriver = nestinglevel + 1;
		    if (indrivers) {
		      if (tagtype == 1) {
			if (debug)
			  fprintf(stderr, 
				  "    Resetting Driver/PPD.\n");
			cid[0] = '\0';
			cppd[0] = '\0';
		      }
		    }
		  } else if (strcmp(currtagname, "drivers") == 0) {
		    indrivers = nestinglevel + 1;
		  } else if (strcmp(currtagname, "id") == 0) {
		    inid = nestinglevel + 1;
		  } else if (strcmp(currtagname, "ppd") == 0) {
		    inppd = nestinglevel + 1;
		  } else if (strcmp(currtagname, "lang") == 0) {
		    inlang = nestinglevel + 1;
		  } else if (strcmp(currtagname, "postscript") == 0) {
		    inpostscript = nestinglevel + 1;
		    if (inlang) {
		      if (tagtype == 1) {
			if (debug)
			  fprintf(stderr, 
				  "    Resetting Driver/PPD.\n");
			cid[0] = '\0';
			cppd[0] = '\0';
		      }
		    }
		  } else if (strcmp(currtagname, "autodetect") == 0) {
		    inautodetect = nestinglevel + 1;
		    if (tagtype == 1) lastautodetect = (char*)lasttag;
		  } else if (strcmp(currtagname, "printer") == 0) {
		    inprinter = nestinglevel + 1;
		    if (tagtype == 1) {
		      /* XML body of the file is starting here */
		      inxmlheader = 0;
		      nestinglevel = 1;
		      /* Remove the whole header of the XML file */
		      if (debug) 
			fprintf(stderr,
				"    Removing XML file header\n");
		      memmove(*data, lasttag, 
			      *data + datalength + 1 - lasttag);
		      datalength -= lasttag - *data;
		      scan -= lasttag - *data;
		      tagwordstart -= lasttag - *data;
		      lasttag = *data;
		      lasttagend = NULL;
		      if (debug) fprintf(stderr, 
					 "    Initializing PPD list.\n");
		      while(ppdlist != NULL) {
			ppdlistpointer = ppdlist;
			ppdlist = (ppdlist_t *)ppdlist->next;
			free(ppdlistpointer);
		      }
		      if (debug) fprintf(stderr, 
					 "    Initializing fields.\n");
		      cprinter[0] = '\0';
		      cmake[0] = '\0';
		      cmodel[0] = '\0';
		      cfunctionality[0] = '\0';
		      cunverified = 0;
		      cdriver[0] = '\0';
		      cautodetectentry[0] = '\0';
		    }
		  }
		}
	      } else { /* additional word = parameter */
		memmove(currtagparam, tagwordstart, scan - tagwordstart);
		currtagparam[scan - tagwordstart] = '\0';
		if (debug) fprintf(stderr, 
				   "    Tag parameter: '%s'\n",
				   currtagparam);
		if (operation == 2) { /* Option XML file */
		  if (strcmp(currtagname, "constraint") == 0) {
		    /* Set the sense of the constraint */
		    if ((s = strstr(currtagparam, "sense")) != NULL) {
		      if (strstr(s + 5, "true") != NULL) {
			csense = 1;
		      } else if (strstr(s + 5, "false") != NULL) {
			csense = 0;
		      }
		    }
		  } else if (strcmp(currtagname, "option") == 0) {
		    if ((s = strstr(currtagparam, "type")) != NULL) {
		      if (strstr(s + 4, "enum") != NULL) {
			/* Set the number of qualified enum values to 0 */
			/* If this value stays 0 we have no enum values */
			/* and the option disqualifies */
			numenumvals = 0;
			optiontype = 0;
		      } else if (strstr(s + 4, "bool") != NULL) {
			optiontype = 1;
		      } else if (strstr(s + 4, "int") != NULL) {
			optiontype = 2;
		      } else if (strstr(s + 4, "float") != NULL) {
			optiontype = 3;
		      }
		    }
		  } else if (strcmp(currtagname, "enum_val") == 0) {
		    if ((s = strstr(currtagparam, "id")) != NULL) {
		      /* Extract the ID of this enum value */
		      strcpy(currevid, s + 4);
		      currevid[strlen(currevid) - 1] = '\0';
		      if (debug)
			fprintf(stderr, 
				"    Enum value ID: '%s'\n",
				currevid);
		    }
		  }
		} else if (operation == 3) { /* Driver XML file (Overview) */
		  if (strcmp(currtagname, "driver") == 0) {
		    if ((s = strstr(currtagparam, "id")) != NULL) {
		      /* Get the short driver name (w/o "driver/") */
		      s = strstr(s + 2, "driver/") + 7;
		      /* Cut off trailing '"' */
		      s[strlen(s)-1] = '\0';
		      strcpy(cdriver, s);
		    }
		  }
		} else if (operation == 4) { /* Printer XML file (Overview) */
		  if (debug)
		    fprintf(stderr, 
			    "    Printer XML file (overview): Tag name: %s, Tag param:%s\n", 
			    currtagname, currtagparam);
		  if (strcmp(currtagname, "printer") == 0) {
		    if ((s = strstr(currtagparam, "id")) != NULL) {
                      /* Get the short printer name (w/o "printer/") */
                      if ((s = strstr(s + 2, "printer/")) != NULL) {
                          s += 8;
                          /* Cut off trailing '"' */
                          s[strlen(s)-1] = '\0';
                          strcpy(cprinter, s);
                      }
		    }
		  }
		}
	      }
	    }
	    if (*scan == '/') { /* tag closing */
	      if (tagnamefound) { /* '/' after tag name, this tag has no 
				     body */
		tagtype = 0;
	      } else { /* we are closing a tag */
		tagtype = -1;
	      }
	      if (debug)
		fprintf(stderr,
			"    End of tag, tag type %d (0: no body, -1: with body)\n", 
			tagtype);
	    }
	  }
	  if (*scan == '>') { /* and of tag */
	    if (incomment) {
	      if ((*(scan - 2) == '-') && (*(scan - 1) == '-')) {
		incomment = 0;
		intag = 0;
		if (debug) fprintf(stderr, "    End comment\n");
	      }
	    } else {
	      intag = 0;
	      if (!inxmlheader && (tagnamefound == 0)) {
		fprintf(stderr, "XML error: Tag without name %s, line %d!\n",
			filename, linecount);
		exit(1);
	      }
	      if (lasttagend != NULL) {
		memmove(currtagbody, lasttagend + 1, lasttag - lasttagend - 1);
		currtagbody[lasttag - lasttagend - 1] = '\0';
		if (debug)
		  fprintf(stderr,
			  "    Contents of tag body: '%s'\n", currtagbody);
	      }
	      nestinglevel += tagtype;
	      /* Important tags end, clear the marks and do appropriate 
		 actions */
	      if (operation == 0) { /* Printer XML file */
		if (nestinglevel < inprinter) inprinter = 0;
		if (nestinglevel < inmake) { /* Found printer manufacturer */
		  inmake = 0;
		  /* Only the <make> outside the <autodetect> tag is valid. */
		  if (!inautodetect) strcat(make, currtagbody);
		}
		if (nestinglevel < inmodel) { /* Found printer model */
		  inmodel = 0;
		  /* Only the <model> outside the <autodetect> tag is valid. */
		  if (!inautodetect) strcat(model, currtagbody);
		}
		if (nestinglevel < inautodetect) inautodetect = 0;
		if (nestinglevel < indrivers) indrivers = 0;
		if (nestinglevel < indriver) {
		  indriver = 0;
		  if (indrivers) {
		    if (debug) fprintf(stderr, 
				       "    Printer/Driver: %s %s\n",
				       pid, cid);
		    if (cid[0] != '\0') {
		      if (debug)
			fprintf(stderr,
				"      Printer XML: Printer: %s Driver: %s\n",
				pid, cid);
		      if (!strcmp(cid, driver)) {
			/* Printer/driver combo already confirmed by
			   <drivers> section of printer XML file */
			if (debug)
			  fprintf(stderr,
				  "      Printer XML: Printer/Driver combo confirmed by <drivers> section!\n");
			comboconfirmed = 1;
		      }
		    }
		  }
		}
		if (cppd[0]) strcpy(cid, "Postscript");
		if (nestinglevel < inid) {
		  inid = 0;
		  strcpy(cid, currtagbody);
		  if (debug) fprintf(stderr, 
				     "    Printer XML: Driver ID: %s\n", cid);
		}
		if (nestinglevel < inlang) inlang = 0;
		if (nestinglevel < inpostscript) {
		  inpostscript = 0;
		  if (inlang) {
		    if (debug) fprintf(stderr, 
				       "    Printer/Driver/PPD: %s %s %s\n",
				       cprinter, cid, cppd);
		  }
		  if (!strcmp(cid, driver)) {
		    /* Printer/driver combo already confirmed by
		       <postscript> section of printer XML file */
		    if (debug)
		      fprintf(stderr,
			      "      Printer XML: Printer/Driver combo confirmed by <postscript> section!\n");
		    comboconfirmed = 1;
		  }
		}
		if (nestinglevel < inppd) {
		  inppd = 0;
		  /* Get the PPD file location without leading 
		     white space, is empty on empty PPD file location*/
		  for (s = currtagbody;
		       (*s != '\0') && (strchr(" \n\r\t", *s) != NULL);
		       s ++);
		  strcpy(cppd, s);
		  if (debug) fprintf(stderr, 
				     "    PPD URL: %s\n", cppd);
		}
	      } else if (operation == 1) { /* Driver XML file */
		if (nestinglevel < inexecution) inexecution = 0;
		if (nestinglevel < innopjl) innopjl = 0;
		if (nestinglevel < indriver) indriver = 0;
		if (nestinglevel < inprinters) {
		  inprinters = 0;
		  /* Remove the whole <printers> block */
		  if (lastprinters != NULL) {
		    if (debug) 
		      fprintf(stderr, "    Removing <printers> block\n");
		    memmove(lastprinters, scan + 1, 
			    *data + datalength - scan);
		    datalength -= scan + 1 - lastprinters;
		    scan = lastprinters - 1;
		    if (debug) 
		      fprintf(stderr, "    Inserting saved printer\n");
		    l = strlen(printerentry);
		    if (l != 0) {
		      memmove(lastprinters + l, lastprinters, 
			      *data + datalength - lastprinters + 1);
		      memmove(lastprinters, printerentry, l);
		      datalength += l;
		      scan += l;
		    }
		  }
		}
		if (nestinglevel < inprinter) {
		  inprinter = 0;
		  if (printertobesaved) {
		    /* Save the printer entry in a buffer to reinsert it
		       after deleting the <printers> block */
		    printertobesaved = 0;
		    if (lastprinter != NULL) {
		      if (debug) fprintf(stderr, "    Saving printer\n");
		      strcat(printerentry,"\n <printers>\n  ");
		      l = strlen(printerentry);
		      memmove(printerentry + l, lastprinter,
			      scan + 1 - lastprinter);
		      printerentry[l + scan + 1 - lastprinter] = '\0';
		      strcat(printerentry,"\n </printers>");
		      if (debug) fprintf(stderr, "    Printer entry %s\n",
					 printerentry);
		    }
		  }
		}
		if (nestinglevel < inid) {
		  inid = 0;
		  /* printer ID after the "printer/" in currtagbody is 
		     used (to not compare the always equal "printer/" */
		  if (strcmp(trpid, 
			     translateid(currtagbody + 8, idlist)) == 0) {
		    /* Found printer entry in driver file */
		    printerentryfound = 1;
		    printertobesaved = 1;
		    if (debug) fprintf(stderr, "    Found printer\n");
		  } else {
		    if (debug) fprintf(stderr, "    Other printer\n");
		  }
		}
	      } else if (operation == 2) { /* Option XML file */
		if ((debug) && (strcmp(currtagname, "constraint") == 0) &&
		    (tagtype == -1)) {
		  j++;
		  fprintf(stderr, "    Constraint %d: %s\n", j, filename);
		}
		if (nestinglevel < inen) {
		  inen = 0;
		  if (inargshortname) {
		    /* We have the short name of the option, check whether
		       the user has defined a default value for it */
		    if (debug)
		      fprintf
			(stderr, "    Option short name: '%s'\n",
			 currtagbody);
		    for (k = 0; k < num_defaultsettings; k ++) {
		      if ((strstr(defaultsettings[k], currtagbody) == 
			   defaultsettings[k]) && 
			  (*(defaultsettings[k] + strlen(currtagbody))
			   == '=')) {
			s = (char *)(defaultsettings[k] +
				     strlen(currtagbody) + 1);
			userdefault = 1;
			if (optiontype == 1) {
			  /* Boolean options */
			  if ((strcasecmp(s, "true") == 0) ||
			      (strcasecmp(s, "yes") == 0) ||
			      (strcasecmp(s, "on") == 0)) {
			    /* "True" */
			    s = "1";
			  } else if ((strcasecmp(s, "false") == 0) ||
				     (strcasecmp(s, "no") == 0) ||
				     (strcasecmp(s, "off") == 0)) {
			    /* "False" */
			    s = "0";
			  } else if ((strcasecmp(s, "0") != 0) &&
				     (strcasecmp(s, "1") != 0)) {
			    /* No valid value for a bool option */
			    userdefault = 0;
			  }
			} else if (optiontype == 2) {
			  /* Integer options */
			  if (strspn(s, "+-0123456789") < strlen(s)) {
			    userdefault = 0;
			  }
			} else if (optiontype == 3) {
			  /* Float options */
			  if (strspn(s, "+-0123456789.eE") < strlen(s)) {
			    userdefault = 0;
			  }
			}
			strcpy(userdefaultvalue, s);
			if ((debug) && (userdefault))
			  fprintf
			    (stderr,
			     "      User default setting: '%s'\n",
			     userdefaultvalue);
		      } else if ((strcmp(defaultsettings[k], currtagbody) ==
				  0) && (optiontype == 1)) {
			/* "True" for boolean options */
			strcpy(userdefaultvalue, "1");
			userdefault = 1;
			if (debug)
			  fprintf
			    (stderr,
			     "      User default setting: '%s'\n",
			     userdefaultvalue);
		      } else if ((strcmp(defaultsettings[k] + 2,currtagbody)
				  == 0) &&
				 (strncasecmp(defaultsettings[k], "no", 2)
				  == 0) && (optiontype == 1)) {
			/* "False" for boolean options */
			strcpy(userdefaultvalue, "0");
			userdefault = 1;
			if (debug)
			  fprintf
			    (stderr,
			     "      User default setting: '%s'\n",
			     userdefaultvalue);
		      }
		    }
		  } else if (inevshortname) {
		    /* We have the short name of the enum value, check
		       whether the user chose this value as default,
		       extract the enum value ID then and mark the user's
		       default value as found */
		    if (debug)
		      fprintf
			(stderr, "    Enum value short name: '%s'\n",
			 currtagbody);
		    if ((userdefault) &&
			(strcmp(userdefaultvalue, currtagbody) == 0)) {
		      strcpy(userdefaultid, currevid);
		      userdefaultfound = 1;
		      if (debug)
			fprintf
			  (stderr,
			   "      User default setting found!\n");
		    }
		  }
		}
		if (nestinglevel < inargmax) {
		  inargmax = 0;
		  if ((optiontype >= 2) && (optiontype <= 3)) {
		    maxnumvalue = atof(currtagbody);
		    if (userdefault) {
		      /* Range-check user default and make it invalid if
			 necessary */
		      if (atof(userdefaultvalue) > maxnumvalue) {
			userdefault = 0;
		      }
		    }
		    if (debug)
		      fprintf
			(stderr,
			 "    Maximum value: %s\n",
			 currtagbody);
		  }
		}
		if (nestinglevel < inargmin) {
		  inargmin = 0;
		  if ((optiontype >= 2) && (optiontype <= 3)) {
		    minnumvalue = atof(currtagbody);
		    if (userdefault) {
		      /* Range-check user default and make it invalid if
			 necessary */
		      if (atof(userdefaultvalue) < minnumvalue) {
			userdefault = 0;
		      }
		    }
		    if (debug)
		      fprintf
			(stderr,
			 "    Minimum value: %s\n",
			 currtagbody);
		  }
		}
		if (nestinglevel < inargshortname) {
		  inargshortname = 0;
		}
		if (nestinglevel < inargexecution) {
		  inargexecution = 0;
		}
		if (nestinglevel < inargpjl) {
		  inargpjl = 0;
		}
		if (nestinglevel < inevshortname) {
		  inevshortname = 0;
		}
		if (nestinglevel < inprinter) {
		  inprinter = 0;
		  if (inconstraint) {
		    /* Make always short printer IDs (w/o "printer/") */
		    if (currtagbody[0] == 'p') {
		      strcat(cprinter, currtagbody + 8);
		    } else {
		      strcat(cprinter, currtagbody);
		    }
		  }
		}
		if (nestinglevel < inmake) {
		  inmake = 0;
		  if (inconstraint) strcat(cmake, currtagbody);
		}
		if (nestinglevel < inmodel) {
		  inmodel = 0;
		  if (inconstraint) strcat(cmodel, currtagbody);
		}
		if (nestinglevel < indriver) {
		  indriver = 0;
		  if (inconstraint) strcat(cdriver, currtagbody);
		}
		if (nestinglevel < inargdefault) {
		  inargdefault = 0;
		  if (inconstraint) strcat(cargdefault, currtagbody);
		}
		if (nestinglevel < inconstraint) {
		  inconstraint = 0;
		  /* Constraint completely read, here we evaluate it */
		  if (debug) {
		    fprintf(stderr,"    Evaluation of constraint\n");
		    fprintf(stderr,"      Values given in constraint:\n");
		    fprintf(stderr,"        make: |%s|, model: |%s|, printer: |%s|\n",
			    cmake, cmodel, cprinter);
		    fprintf(stderr,"        driver: |%s|, argdefault: |%s|, sense: |%d|\n",
			    cdriver, cargdefault, csense);
		    fprintf(stderr,"      Values of current printer/driver combo:\n");
		    fprintf(stderr,"        make: |%s|, model: |%s|\n",
			    make, model);
		    fprintf(stderr,"        PID: |%s|, driver: |%s|\n",
			    pid, driver);
		  }
		  if (!((cmake[0]) || (cmodel[0]) || 
			(cprinter[0]) || (cdriver[0]))) {
		    fprintf(stderr, "WARNING: Illegal null constraint in %s, line %d!\n",
			    filename, linecount);
		  } else if (((cmake[0]) || (cmodel[0])) && (cprinter[0])) {
		    fprintf(stderr, "WARNING: Both printer id and make/model in constraint in %s, line %d!\n",
			    filename, linecount);
		  } else {
		    if (debug) 
		      fprintf(stderr,
			      "      Highest scores for printer: |%d|, driver: |%d|\n",
			      printerhiscore, driverhiscore);
                    /* if make matches, printerscore match grade 1 */
                    /* if model matches, printerscore match grade 2 */
                    /* no information, printerscore = 0 */
		    /* mismatch, printerscore = -1 */
		    printerscore = 0;
		    /* driverscore: -1 = mismatch, 1 = match, 0 = no info */
		    driverscore = 0;
		    /* The per-printer constraining can happen by poid or by
		       a make[/model] pair */
		    if (cprinter[0]) {
		      if (debug) fprintf(stderr,"        Checking PID\n");
		      if (strcmp(translateid(cprinter, idlist), trpid) == 0)
			printerscore = 2;
		      else
			printerscore = -1;
		    } else if (cmake[0]) {
		      if (debug) fprintf(stderr,"        Checking make\n");
		      /* We have a requested make, so it can't be zero.
			 You can't request or constraint by model only! */
		      if (strcmp(cmake, make) == 0) {
			printerscore = 1; /* make matches */
			if (cmodel[0]) {
			  if (debug)
			    fprintf(stderr,"        Checking model\n");
			  if (strcmp(cmodel, model) == 0)
			    printerscore = 2; /* model matches, too */
			  else
			    printerscore = -1; /* model mismatch */
			}
		      } else printerscore = -1; /* make mismatch */
		    }
		    /* Is a driver requested? */
		    if (cdriver[0]) {
		      if (debug) 
			fprintf(stderr,"        Checking driver\n");
		      if ((strcmp(cdriver, driver) == 0) ||
			  (strcmp(cdriver + 7, driver) == 0)) 
			driverscore = 1; /* driver matches */
		      else
			driverscore = -1; /* driver mismatch */
		    }
		    if (debug)
		      fprintf(stderr,
			      "      Scores for this constraint: printer: |%d|, driver: |%d|\n",
			      printerscore, driverscore);
		    /* Now compare the scores with the ones of the currently
		       best-matching constraint */
		    /* Any sort of match? */
		    if (((printerscore > 0) || (driverscore > 0)) &&
			((printerscore > -1) && (driverscore > -1))) {
		      if (debug) fprintf(stderr,
					 "      Something matches\n");
		      /* Does this beat the best match to date? */
		      if (((printerscore >= printerhiscore) &&
			   (driverscore >= driverhiscore)) ||
			  /* They're equal or better in both categories */
			  (printerscore == 2)) {
			  /* A specific printer always wins */
			if (debug)
			  fprintf(stderr,"      This constraint wins\n");
			/* Set the high scores */
			if (printerscore > printerhiscore) {
			  printerhiscore = printerscore;
			}
			if (driverscore > driverhiscore) {
			  driverhiscore = driverscore;
			}
			/* Constraint applies */
			if (inenumval) {
			  /* The winning constraint determines with its
			     sense whether the option/the enum value
			     qualifies for our printer/driver combo */
			  enumvalqualified = csense;
			  if (debug) 
			    fprintf(stderr,
				    "      Enumeration choice qualifies? %d (0: No, 1: Yes)\n",
				    enumvalqualified);
			} else {
			  optionqualified = csense;
			  if (debug) 
			    fprintf(stderr,
				    "      Option qualifies? %d (0: No, 1: Yes)\n",
				    optionqualified);
			  /* The winning constraint for the option
			     determines the default setting for this
			     option */
			  strcpy(argdefault, cargdefault);
			}
		      }
		    }
		  }
		}
		if (nestinglevel < inconstraints) {
		  inconstraints = 0;
		  /* End of <constraints> block, did the option/the enum
		     value qualify for our printer/driver combo? */
		  if (inenumval) {
		    if (debug)
		      fprintf(stderr,
			      "    This enumeration value finally qualified? %d (0: No, 1: Yes)\n",
			      enumvalqualified);
		    if (!enumvalqualified) enumvaltoberemoved = 1;
		  } else {
		    if (debug)
		      fprintf(stderr,
			      "    This option finally qualified?  %d (0: No, 1: Yes)\n",
			      optionqualified);
		    if (!optionqualified) {
		      /* We have reached the end of the <constraints> block
			 for this option, and the option's constraints 
			 didn't qualify the option for our printer/driver
			 combo =>
			 remove this option file from memory and return. */
		      free((void *)(*data));
		      *data = NULL;
		      if (debug)
			fprintf(stderr, "    Option does not apply!\n");
		      return;
		    }
		  }
		  if (debug)
		    fprintf(stderr,
			    "    Constr. for enum. value? %d, enum value disqualified? %d (0: No, 1: Yes)\n",
			    inenumval, enumvaltoberemoved);
		  if ((!inenumval) || (!enumvaltoberemoved)) {
		    /* Remove the read <constraints> block, it will not
		       appear in the output, but don't remove it if the
		       current enum value will be removed anyway */
		    if (lastconstraints != NULL) {
		      if (debug)
			fprintf(stderr, "    Removing constraints block\n");
		      memmove(lastconstraints, scan + 1, 
			      *data + datalength - scan);
		      datalength -= scan + 1 - lastconstraints;
		      scan = lastconstraints - 1;
		    } else {
		      if (debug)
			fprintf(stderr, "    This enum value will be removed anyway, so constraints block does not  \n    need to be removed.\n");
		    }
		  }
		}
		if (nestinglevel < inenumval) {
		  inenumval = 0;
		  if (debug) 
		    fprintf(stderr,
			    "    End of enumeration value block, to be removed? %d (0: No, 1: Yes)\n",
			    enumvaltoberemoved);
		  if (enumvaltoberemoved) {
		    /* This enum value does not apply to our printer/driver
		       combo, remove it */
		    if (lastenumval != NULL) {
		      if (debug) fprintf(stderr, "    Removing enumeration value\n");
		      memmove(lastenumval, scan + 1, 
			      *data + datalength - scan);
		      datalength -= scan + 1 - lastenumval;
		      scan = lastenumval - 1;
		    } else {
		      fprintf (stderr, "    Cannot remove this evaluation value.\n");
		    }
		  } else
		    /* This enum value applies to our printer/driver combo */
		    numenumvals++;
		}
		if (nestinglevel < inoption) {
		  inoption = 0;
		  if (debug)
		    fprintf(stderr,
			    "End of option block:\n      No. of enum. values: %d, qualified by constraints? %d (0: No, 1: Yes)\n",
			    numenumvals, optionqualified);
		  if ((!numenumvals) || (!optionqualified)) {
		    /* We have reached the end of the option file, but there
                       are no enum values which qualified for our combo
		       or there were no constraints at all =>
		       remove this option file from memory and return. */
		    free((void *)(*data));
		    *data = NULL;
		    if (debug) fprintf (stderr, "    No enum. values, no constraints => Removing option!\n");
		    return;
		  }
		  /* Insert the line determining the default setting */
		  if ((argdefault[0]) || (userdefault)) {
		    if (lastoption != NULL) {
		      if (debug) 
			fprintf(stderr, 
				"    Inserting default value\n");
		      if (userdefault) {
			/* There is a user-defined default setting */
			if ((optiontype >= 1) && (optiontype <= 3)) {
			  /* Boolean or numerical option */
			  strcpy(argdefault, userdefaultvalue);
			} else {
			  /* enumerated option */
			  if (userdefaultfound) {
			    strcpy(argdefault, userdefaultid);
			  }
			}
		      }
		      sprintf(defaultline,
			      "\n  <arg_defval>%s</arg_defval>",
			      argdefault);
		      defaultlinelength = strlen(defaultline);
		      memmove(lastoption + defaultlinelength, lastoption, 
			      *data + datalength - lastoption + 1);
		      memmove(lastoption, defaultline, defaultlinelength);
		      datalength += defaultlinelength;
		      scan += defaultlinelength;
		      if (debug) 
			fprintf(stderr,
				"      Default value line: %s\n",
				defaultline);
		    }
		  }
		}
	      } else if (operation == 3) { /* Driver XML file (Overview) */
		if (nestinglevel < indriver) indriver = 0;
		if (nestinglevel < inprinters) {
		  inprinters = 0;
		  /* Remove the whole <printers> block */
		  if (lastprinters != NULL) {
		    if (debug) 
		      fprintf(stderr, "    Removing <printers> block\n");
		    memmove(lastprinters, scan + 1, 
			    *data + datalength - scan);
		    datalength -= scan + 1 - lastprinters;
		    scan = lastprinters - 1;
		  }
		}
		if (nestinglevel < incomments) {
		  incomments = 0;
		  /* Remove the whole <comments> block */
		  if ((lastcomments != NULL) && (!inprinter)) {
		    if (debug) 
		      fprintf(stderr, "    Removing <comments> block\n");
		    memmove(lastcomments, scan + 1, 
			    *data + datalength - scan);
		    datalength -= scan + 1 - lastcomments;
		    scan = lastcomments - 1;
		  }
		}
		if (nestinglevel < inexecution) inexecution = 0;
		if (nestinglevel < inid) {
		  inid = 0;
		  /* Get the short printer ID (w/o "printer/") */
		  strcpy(cprinter, currtagbody + 8);
		  strcpy(cprinter, translateid(cprinter, idlist));
		  if (debug)
		    fprintf(stderr,
			    "    Overview: Printer: %s Driver: %s\n",
			    cprinter, cdriver);
		}
		if (nestinglevel < indfunctionality) {
		  indfunctionality = 0;
		  /* Save the functionality entry in a buffer to insert it
		     into the overview */
		  if (lastdfunctionality != NULL) {
		    if (debug) fprintf(stderr,
				       "    Saving <functionality> entry\n");
		    memmove(dfunctionalityentry, lastdfunctionality,
			    scan + 1 - lastdfunctionality);
		    dfunctionalityentry[scan + 1 - lastdfunctionality] = '\0';
		    if (debug) fprintf(stderr,
				       "    <functionality> entry: %s\n",
				       dfunctionalityentry);
		  }
		}
		if (nestinglevel < inprinter) {
		  inprinter = 0;
		  if (debug)
		    fprintf(stderr,
			    "    Overview: Add driver %s to printer %s (%s)\n",
			    cdriver, cprinter, dfunctionalityentry);
		  /* Add this driver to the current printer's entry in the
		     printer list, create the printer entry if necessary */
		  plistpointer = *printerlist;
		  plistpreventry = NULL;
		  /* Search printer in list */
		  while ((plistpointer != NULL) &&
			 (strcmp(plistpointer->id, cprinter) != 0)) {
		    plistpreventry = plistpointer;
		    plistpointer = (printerlist_t *)(plistpointer->next);
		  }
		  if (plistpointer == NULL) {
		    /* printer not found, create new entry */
		    plistpointer = 
		      (printerlist_t *)malloc(sizeof(printerlist_t));
		    strcpy(plistpointer->id, cprinter);
		    plistpointer->drivers = NULL;
		    plistpointer->next = NULL;
		    if (plistpreventry != NULL)
		      plistpreventry->next = 
			(struct printerlist_t *)plistpointer;
		    else 
		      *printerlist = plistpointer;
		  }
		  /* Add driver entry */
		  dlistpointer = plistpointer->drivers;
		  dlistpreventry = NULL;
		  while (dlistpointer != 0) {
		    dlistpreventry = dlistpointer;
		    dlistpointer = (driverlist_t *)(dlistpointer->next);
		  }
		  dlistpointer = 
		    (driverlist_t *)malloc(sizeof(driverlist_t));
		  strcpy(dlistpointer->name, cdriver);
		  if ((dfunctionalityentry != NULL) &&
		      (dfunctionalityentry[0]))
		    dlistpointer->functionality = strdup(dfunctionalityentry);
		  else dlistpointer->functionality = NULL;
		  dlistpointer->next = NULL;
		  if (dlistpreventry != NULL)
		    dlistpreventry->next =
		      (struct driverlist_t *)dlistpointer;
		  else 
		    plistpointer->drivers = dlistpointer;
		}
		if (nestinglevel < inprototype) {
		  inprototype = 0;
		  if (pid) { /* We abuse pid here to tell that we want
				to have an overview of available PPDs and
				not of all possible printer/driver combos.
			        pid is never used for a printer ID when 
			        building the overview XML file. */
		    /* Get the command line prototype without leading 
                       white space, is empty on empty command line*/
		    for (s = currtagbody;
			 (*s != '\0') && (strchr(" \n\r\t", *s) != NULL);
			 s ++);
		    if (debug)
		      fprintf(stderr,
			      "    Overview: Driver: %s Command line: |%s|\n",
			      cdriver, s);
		    if (*s == '\0') {
		      /* We have found an empty command line prototype, so]
			 this driver does not produce any PPD file, 
			 mark this driver as not having a command line
			 prototype, remove the file from memory and
			 return. */
		      /* Add the driver to the first entry in the printer
			 list, the pseudo printer "noproto" */
		      plistpointer = *printerlist;
		      plistpreventry = NULL;
		      dlistpointer = plistpointer->drivers;
		      dlistpreventry = NULL;
		      while ((dlistpointer != 0) &&
			     (strcasecmp(dlistpointer->name, cdriver))) {
			dlistpreventry = dlistpointer;
			dlistpointer = (driverlist_t *)(dlistpointer->next);
		      }
		      if (dlistpointer == 0) {
			dlistpointer = 
			  (driverlist_t *)malloc(sizeof(driverlist_t));
			strcpy(dlistpointer->name, cdriver);
			dlistpointer->functionality = NULL;
			dlistpointer->next = NULL;
			if (dlistpreventry != NULL)
			  dlistpreventry->next =
			    (struct driverlist_t *)dlistpointer;
			else 
			  plistpointer->drivers = dlistpointer;
		      }
		      /* Renove the driver XML data from memory */
		      free((void *)(*data));
		      *data = NULL;
		      if (debug)
			fprintf(stderr, "    Driver entry does not produce PPDs!\n");
		      return;
		    }
		  }
		  /* Remove the whole <prototype> block */
		  if (lastprototype != NULL) {
		    if (debug) 
		      fprintf(stderr, "    Removing <prototype> block\n");
		    memmove(lastprototype, scan + 1, 
			    *data + datalength - scan);
		    datalength -= scan + 1 - lastprototype;
		    scan = lastprototype - 1;
		  }
		}
	      } else if (operation == 4) { /* Printer XML file (Overview) */
		if (debug)
		  fprintf(stderr,
			  "    Printer (Overview), tag name: %s, tag body: %s\n",
			  currtagname, currtagbody);
		if (nestinglevel < inprinter) inprinter = 0;
		if (nestinglevel < inmake) {
		  inmake = 0;
		  /* Only the <make> outside the <autodetect> tag is valid. */
		  if (!inautodetect) strcpy(cmake, currtagbody);
		}
		if (nestinglevel < inmodel) {
		  inmodel = 0;
		  /* Only the <model> outside the <autodetect> tag is valid. */
		  if (!inautodetect) strcpy(cmodel, currtagbody);
		}
		if (nestinglevel < infunctionality) {
		  infunctionality = 0;
		  strcpy(cfunctionality, currtagbody);
		}
		if (nestinglevel < inunverified) inunverified = 0;
		if (nestinglevel < indrivers) indrivers = 0;
		if (nestinglevel < inlang) inlang = 0;
		if ((nestinglevel < indriver) ||
		    (nestinglevel < inpostscript)) {
		  if (nestinglevel < indriver) indriver = 0;
		  if (nestinglevel < inpostscript) inpostscript = 0;
		  if (indrivers || inlang) {
		    if (debug) fprintf(stderr, 
				       "    Printer/Driver/PPD: %s %s %s\n",
				       cprinter, cid, cppd);
		    driverhasproto = 0;
		    if ((cid[0] != '\0') && (pid)) {
		      /* Check if our driver has a command line prototype,
			 it should not be driver of the pseudo-printer
			 "noproto" (first item in the printer list) then */
		      plistpointer = *printerlist;
		      plistpreventry = NULL;
		      dlistpointer = plistpointer->drivers;
		      dlistpreventry = NULL;
		      while ((dlistpointer != 0) &&
			     (strcasecmp(dlistpointer->name, cid))) {
			dlistpreventry = dlistpointer;
			dlistpointer = (driverlist_t *)(dlistpointer->next);
		      }
		      if (dlistpointer == 0) {
			driverhasproto = 1;
		      }
		    }
		    if ((cid[0] != '\0') && 
			((!pid) || /* We want to see all combos, not only
				      the ones which provide a PPD file
				      If pid is set, we want only combos
				      which provide PPDs and if we do
				      not have a ready-made PPD we must */
			 (driverhasproto) || /* have a command line
						prototype */
			 ((cppd[0] != '\0')))) { /* or a ready-made
						    PPD file. */
		      if (debug)
			fprintf(stderr,
				"    Overview: Printer: %s Driver: %s\n",
				cprinter, cid);
		      /* Add this driver to the current printer's entry in 
			 the printer list, create the printer entry if
			 necessary */
		      plistpointer = *printerlist;
		      plistpreventry = NULL;
		      /* Search printer in list */
		      while ((plistpointer != NULL) &&
			     (strcmp(plistpointer->id, cprinter) != 0)) {
			plistpreventry = plistpointer;
			plistpointer = (printerlist_t *)(plistpointer->next);
		      }
		      if (plistpointer == NULL) {
			/* printer not found, create new entry */
			plistpointer = 
			  (printerlist_t *)malloc(sizeof(printerlist_t));
			strcpy(plistpointer->id, cprinter);
			plistpointer->drivers = NULL;
			plistpointer->next = NULL;
			if (plistpreventry != NULL)
			  plistpreventry->next = 
			    (struct printerlist_t *)plistpointer;
			else 
			  *printerlist = plistpointer;
		      }
		      /* Add driver entry */
		      dlistpointer = plistpointer->drivers;
		      dlistpreventry = NULL;
		      while ((dlistpointer != 0) &&
			     (strcasecmp(dlistpointer->name, cid))) {
			dlistpreventry = dlistpointer;
			dlistpointer = (driverlist_t *)(dlistpointer->next);
		      }
		      if (dlistpointer == 0) {
			dlistpointer = 
			  (driverlist_t *)malloc(sizeof(driverlist_t));
			strcpy(dlistpointer->name, cid);
			dlistpointer->functionality = NULL;
			dlistpointer->next = NULL;
			if (dlistpreventry != NULL)
			  dlistpreventry->next =
			    (struct driverlist_t *)dlistpointer;
			else 
			  plistpointer->drivers = dlistpointer;
		      }
		    }
		    if ((cid[0] != '\0') && (cppd[0] != '\0')) {
		      if ((pid == NULL) || (pid[0] == 'C')) {
			/* CUPS should also show entries for ready-made 
			   PPD files in the Foomatic database */
			if (debug)
			  fprintf(stderr, 
				  "    Adding Driver/PPD to list.\n");
			ppdlistpointer = ppdlist;
			ppdlistpreventry = NULL;
			if (debug)
			  fprintf(stderr,
				  "    Going through list: ");
			while (ppdlistpointer != NULL) {
			  ppdlistpreventry = ppdlistpointer;
			  ppdlistpointer = (ppdlist_t *)ppdlistpointer->next;
			  if (debug)
			    fprintf(stderr,
				    ".");
			}
			ppdlistpointer = 
			  (ppdlist_t *)malloc(sizeof(ppdlist_t));
			strcpy(ppdlistpointer->driver, cid);
			strcpy(ppdlistpointer->ppd, cppd);
			ppdlistpointer->next = NULL;
			if (ppdlistpreventry != NULL)
			  ppdlistpreventry->next =
			    (struct ppdlist_t *)ppdlistpointer;
			else 
			  ppdlist = ppdlistpointer;
			if (debug)
			  fprintf(stderr,
				  " Driver/PPD in list: %s %s\n",
				  ppdlistpointer->driver,
				  ppdlistpointer->ppd);
		      } else if ((pid != NULL) && (pid[0] == 'c')) {
			/* CUPS should not show entries for ready-made 
			   PPD files in the Foomatic database */
			/* To suppress the printer/driver combo from the output
			   list we need to delete the appropriate driver 
			   entry */
			plistpointer = *printerlist;
			plistpreventry = NULL;
			/* Search printer in list */
			while ((plistpointer != NULL) &&
			       (strcmp(plistpointer->id, cprinter) != 0)) {
			  plistpreventry = plistpointer;
			  plistpointer = (printerlist_t *)(plistpointer->next);
			}
			/* If the printer is there, search for the driver */
			if (plistpointer != NULL) {
			  dlistpointer = plistpointer->drivers;
			  dlistpreventry = NULL;
			  while ((dlistpointer != NULL) &&
				 (strcasecmp(dlistpointer->name, cid))) {
			    dlistpreventry = dlistpointer;
			    dlistpointer =
			      (driverlist_t *)(dlistpointer->next);
			  }
			  if (dlistpointer != NULL) {
			    dlistnextentry =
			      (driverlist_t *)(dlistpointer->next);
			    free(dlistpointer);
			    if (dlistpreventry != NULL)
			      dlistpreventry->next = 
				(struct driverlist_t *)dlistnextentry;
			    else
			      plistpointer->drivers = dlistnextentry;
			  }
			}
		      }
		    }
		  } else strcpy(cdriver, currtagbody);
		}
		if (nestinglevel < inid) {
		  inid = 0;
		  strcpy(cid, currtagbody);
		  if (debug) fprintf(stderr, 
				     "    Driver ID for PPD: %s\n", cid);
		}
		if (nestinglevel < inppd) {
		  inppd = 0;
		  /* Get the PPD file location without leading 
		     white space, is empty on empty PPD file location*/
		  for (s = currtagbody;
		       (*s != '\0') && (strchr(" \n\r\t", *s) != NULL);
		       s ++);
		  strcpy(cppd, s);
		  if (debug) fprintf(stderr, 
				     "    PPD URL: %s\n", cppd);
		}
		if (nestinglevel < inautodetect) {
		  inautodetect = 0;
		  /* Save the autodetect entry in a buffer to insert it
		     into the overview */
		  if (lastautodetect != NULL) {
		    if (debug) fprintf(stderr,
				       "    Saving <autodetect> entry\n");
		    memmove(cautodetectentry, lastautodetect,
			    scan + 1 - lastautodetect);
		    cautodetectentry[scan + 1 - lastautodetect] = '\0';
		    if (debug) fprintf(stderr,
				       "    <autodetect> entry: %s\n",
				       cautodetectentry);
		  }
		}
	      }
	      lasttagend = scan;
	      if (debug) fprintf(stderr,
				 "    XML tag nesting level: %d\n",
				 nestinglevel);
	    }
	  }
	} else {
	  if ((*scan == '>') && 0) { /* tag end without beginning */
	    fprintf(stderr, "XML error: '>' without '<' %s, line %d!\n",
		    filename, linecount);
	    exit(1);
	  }
	}
      }
      break;
    default:
      /* other characters */
      if ((intag) && (!incomment)) {
	if (*scan == '\'') insinglequotes = 1 - insinglequotes;
	if (*scan == '\"') indoublequotes = 1 - indoublequotes;
	if ((insinglequotes) || (indoublequotes)) inquotes = 1;
	else inquotes = 0; 
	if (!incomment) {
	  if (!intagword) {
	    intagword = 1;
	    tagwordstart = scan;
	  }
	}
      }
    }
  }
  if (debug) fprintf(stderr, 
		     "    XML tag nesting level: %d\n",nestinglevel);
  if (debug) fprintf(stderr, "    Lines of input: %d\n", linecount);
  if (operation == 0) { /* Printer XML file */
    if ((make[0] == 0) || (model[0] == 0)) {
      /* <make> or <model> tag not found */
      fprintf(stderr, "Could not determine manufacturer or model name from the printer file %s!\n",
	      filename);
      exit(1);
    }
    if (debug) fprintf(stderr, "    Driver in printer's driver list: %d\n", comboconfirmed); 
  } else if (operation == 1) { /* Driver XML file */
    if (debug) 
      fprintf(stderr,
	      "    nopjl: %d (1: driver does not allow PJL options)\n",
	      *nopjl);
    if (printerentryfound != 0) { /* the printer is in the listing of the 
				     driver */
      comboconfirmed = 1;
    }
    if (debug) fprintf(stderr, "    Printer in driver's printer list: %d\n", comboconfirmed); 
  } else if (operation == 4) { /* Printer XML file (Overview) */
    /* Remove the printer input data */
    **data = '\0';
    /* Build the printer entry for the overview in the memory which was used
       for the former input data, the overview entry is always shorter than
       the original printer XML file. */
    if (debug) fprintf(stderr, "    Data for this printer entry in the overview:\n      Printer ID: |%s|\n      Make: |%s|\n      Model: |%s|\n      Functionality: |%s|\n      Rec. driver: |%s|\n      Auto detect entry: |%s|\n",
	    cprinter, cmake, cmodel, cfunctionality, cdriver,cautodetectentry);
    if ((cprinter[0]) && (cmake[0]) && (cmodel[0]) && (cfunctionality[0])) {
      strcpy(cprinter, translateid(cprinter, idlist));
      strcat((char *)(*data), "  <printer>\n    <id>");
      strcat((char *)(*data), cprinter);
      strcat((char *)(*data), "</id>\n    <make>");
      strcat((char *)(*data), cmake);
      strcat((char *)(*data), "</make>\n    <model>");
      strcat((char *)(*data), cmodel);
      strcat((char *)(*data), "</model>\n    <functionality>");
      strcat((char *)(*data), cfunctionality);
      strcat((char *)(*data), "</functionality>\n");
      if (cunverified) {
	strcat((char *)(*data), "    <unverified>");
	strcat((char *)(*data), cfunctionality);
	strcat((char *)(*data), "</unverified>\n");
      }
      if (cdriver[0]) {
	strcat((char *)(*data), "    <driver>");
	strcat((char *)(*data), cdriver);
	strcat((char *)(*data), "</driver>\n");
      }
      if (cautodetectentry[0]) {
	strcat((char *)(*data), "    ");
	strcat((char *)(*data), cautodetectentry);
      }
      plistpointer = *printerlist;
      plistpreventry = NULL;
      while ((plistpointer) &&
             (strcmp(plistpointer->id, cprinter) != 0)) {
	plistpreventry = plistpointer;
	plistpointer = (printerlist_t *)(plistpointer->next);
      }
      if (plistpointer) {
	strcat((char *)(*data), "\n    <drivers>\n");
	dlistpointer = plistpointer->drivers;
	exceptionfound = 0;
	while (dlistpointer) {
	  strcat((char *)(*data), "      <driver>");
	  strcat((char *)(*data), dlistpointer->name);
	  strcat((char *)(*data), "</driver>\n");
	  if (dlistpointer->functionality != NULL) exceptionfound = 1;
	  dlistpointer = (driverlist_t *)(dlistpointer->next);
	}
	strcat((char *)(*data), "    </drivers>\n");
	if (exceptionfound) {
	  strcat((char *)(*data), "    <driverfunctionalityexceptions>\n");
	  dlistpointer = plistpointer->drivers;
	  while (dlistpointer) {
	    if ((dlistpointer->name != NULL) &&
		(dlistpointer->functionality != NULL)) {
	      strcat((char *)(*data),
		     "      <driverfunctionalityexception>\n");
	      strcat((char *)(*data), "        <driver>");
	      strcat((char *)(*data), dlistpointer->name);
	      strcat((char *)(*data), "</driver>\n");
	      strcat((char *)(*data), dlistpointer->functionality);
	      strcat((char *)(*data),
		     "\n      </driverfunctionalityexception>\n");
	    }
	    dlistpointer = (driverlist_t *)(dlistpointer->next);
	  }
	  strcat((char *)(*data), "    </driverfunctionalityexceptions>\n");
	}
	/* We remove every printer entry in the list for which we have found
	   a printer XML file in the database, so all remaining entries are
	   of printers which are only mentioned in a driver's printer list
	   but do not have an XML file in the database. We will treat these
	   printers later */
	dlistpointer = plistpointer->drivers;
	while (dlistpointer) {
	  dlistpreventry = dlistpointer;
	  dlistpointer = (driverlist_t *)(dlistpointer->next);
	  free(dlistpreventry);
	}
	if (plistpreventry == NULL)
	  *printerlist = (printerlist_t *)(plistpointer->next);
	else
	  plistpreventry->next = plistpointer->next;
	free(plistpointer);
      }
      if (ppdlist != NULL) {
	strcat((char *)(*data), "    <ppds>\n");
	ppdlistpointer = ppdlist;
	if (debug)
	  fprintf(stderr,
		  "    Going through list: ");
	while (ppdlistpointer) {
	if (debug)
	  fprintf(stderr,
		  ".");
	  strcat((char *)(*data), "      <ppd>\n");
	  strcat((char *)(*data), "        <driver>");
	  strcat((char *)(*data), ppdlistpointer->driver);
	  strcat((char *)(*data), "</driver>\n        <ppdfile>");
	  strcat((char *)(*data), ppdlistpointer->ppd);
	  strcat((char *)(*data), "</ppdfile>\n");
	  strcat((char *)(*data), "      </ppd>\n");
	  ppdlistpointer = (ppdlist_t *)(ppdlistpointer->next);
	}
	strcat((char *)(*data), "    </ppds>\n");
      }
      strcat((char *)(*data), "  </printer>\n");
    }
  } else if (operation == 2) { /* Option XML file */
    if (debug) fprintf(stderr, "    Resulting option XML:\n%s\n", *data); 
  }
  return(comboconfirmed);
}

/*
 *  Main function
 */

int                 /* O - Exit status of the program */
main(int  argc,     /* I - Number of command-line arguments */
     char *argv[])  /* I - Command-line arguments */
{
  int		i,j;		/* Looping vars */
  const char    *tmpstr1;       /* Temporary string constant */
  char          *t;

  const char    *pid = NULL,
                *driver = NULL,
                *setting = NULL;/* User-supplied data */
  const char    *make, *model;  /* For constraints */
  const char    *libdir = NULL; /* Database location */
  char          printerfilename[1024];/* Name of printer's XML file */
  char          printerdirname[1024]; /* Name of the directory with the XML
					 files for the printers */
  char          driverfilename[1024]; /* Name of driver's XML file */
  char          driverdirname[1024];  /* Name of the directory with the XML
					 files for the drivers */
  char          optionfilename[1024]; /* Name of current option XML file*/
  char          optiondirname[1024];  /* Name of the directory with the XML
					 files for the options */
  char          oldidfilename[1024];  /* Name of the file with the
					 translation table for old printer
					 IDs */
  char          *printerbuffer = NULL;
  char          *driverbuffer = NULL;
  char          **optbuffers = NULL;
  int           num_optbuffers = 0;
  char          **defaultsettings = NULL; /* User-supplied option settings*/
  int           num_defaultsettings = 0;
  int           overview = 0;
  int           noreadymadeppds = 0;
  int           nopjl = 0;
  int           debug = 0;
  int           debug2 = 0;
  int           comboconfirmed = 0;
  int           comboconfirmed2 = 0;
  int           exceptionfound = 0;
  DIR           *optiondir;
  DIR           *driverdir;
  DIR           *printerdir;
  struct dirent *direntry;
  printerlist_t *printerlist = NULL;
  printerlist_t *plistpointer;  /* pointers to navigate through the 
				   printer */
  driverlist_t  *dlistpointer;  /* list for the overview */
  printerlist_t *plistpreventry;
  driverlist_t  *dlistpreventry;
  idlist_t      *idlist;        /* I - ID translation table */
  
  /* Show the help message whem no command line arguments are given */

  if (argc < 2) {
    fprintf(stderr, "Usage: foomatic-combo-xml [ -O ] [ -p printer -d driver ]\n                          [ -o option1=setting1 ] [ -o option2 ] [ -l dir ]\n                          [ -v | -vv ]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   -p printer   Foomatic ID of the printer\n");
    fprintf(stderr, "   -d driver    Name of the driver to use\n");
    fprintf(stderr, "   -o option1=setting1\n");
    fprintf(stderr, "   -o option2   Default option settings for the\n");
    fprintf(stderr, "                generated file\n");
    fprintf(stderr, "   -O           Generate overview XML file\n");
    fprintf(stderr, "   -C           Generate overview XML file only\n");
    fprintf(stderr, "                containing combos leading to a valid\n");
    fprintf(stderr, "                PPD file (for CUPS PPD list)\n");
    fprintf(stderr, "   -n           (used only with \"-C\") suppress the\n");
    fprintf(stderr, "                printer/driver combos which point to \n");
    fprintf(stderr, "                ready-made PPD file (CUPS usually \n");
    fprintf(stderr, "                lists ready-made PPD files directly).\n");
    fprintf(stderr, "   -l dir       Directory where the Foomatic database is located\n");
    fprintf(stderr, "   -v           Verbose (debug) mode\n");
    fprintf(stderr, "   -vv          Very Verbose (debug) mode\n");
    fprintf(stderr, "\n");
    exit(1);
  }

  /* Read the command line arguments */
  for (i = 1; i < argc; i ++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 'P' :
	case 'p' : /* printer */
	    if (argv[i][2] != '\0')
	      pid = argv[i] + 2;
	    else {
	      i ++;
	      pid = argv[i];
	    }
	    break;
	case 'd' : /* driver */
	    if (argv[i][2] != '\0')
	      driver = argv[i] + 2;
	    else {
	      i ++;
	      driver = argv[i];
	    }
	    break;
	case 'o' : /* option setting */
	    if (argv[i][2] != '\0')
	      setting = argv[i] + 2;
	    else {
	      i ++;
	      setting = argv[i];
	    }
	    num_defaultsettings ++;
	    defaultsettings = (char **)realloc((char **)defaultsettings, 
				      sizeof(char *) * num_defaultsettings);
	    defaultsettings[num_defaultsettings-1] = strdup(setting);
	    break;
	case 'O' : /* Overview */
	    overview = 1;
	    break;
	case 'C' : /* Overview for CUPS PPD list */
	    overview = 2;
	    break;
	case 'n' : /* suppress ready-made PPDs in overview for CUPS PPD list */
	    noreadymadeppds = 1;
	    break;
        case 'l' : /* libdir */
	    if (argv[i][2] != '\0')
	      libdir = argv[i] + 2;
	    else {
	      i ++;
	      libdir = argv[i];
	    }
	    break;
        case 'v' : /* verbose */
	    debug++;
	    if (argv[i][2] == 'v') debug++;
	    break;
	default :
	    fprintf(stderr, "Unknown option \'%c\'!", argv[i][1]);
            exit(1);
      }
    } else {
      fprintf(stderr, "Unknown argument \'%s\'!", argv[i]);
    }
  }

  /* Very verbose debug mode */

  if (debug > 1) debug2 = 1;

  /* Set libdir to the default if empty */

  if (libdir == NULL)
    libdir = "/usr/share/foomatic";

  /* Load translation table for old printer IDs */
  sprintf(oldidfilename, "%s/db/oldprinterids",
	  libdir);
  idlist = loadidlist(oldidfilename);
  if (debug) {
    if (idlist) {
      fprintf(stderr, "Printer ID translation table loaded!\n");
    } else {
      fprintf(stderr,
     "Printer ID translation table corrupted, missing, or not readable!\n");
    }
  }

  if (!overview) {

    /*
     * Compute combo XML file for a given printer/driver combo
     */

    /* Check user-supplied parameters */

    if (pid == NULL) {
      fprintf(stderr, "A printer ID must be supplied!\n");
      exit(1);
    }
    if (driver == NULL) {
      fprintf(stderr, "A driver name must be supplied!\n");
      exit(1);
    }
    
    /* Set file/dir names */
    
    sprintf(printerfilename, "%s/db/source/printer/%s.xml",
	    libdir, pid);
    sprintf(driverfilename, "%s/db/source/driver/%s.xml",
	    libdir, driver);
    sprintf(optiondirname, "%s/db/source/opt",
	    libdir);
    
    /* Read the printer file and extract the printer manufacturer and 
       model */

    if (debug) fprintf(stderr, "Printer file: %s\n", printerfilename);
    printerbuffer = loadfile(printerfilename);
    if (printerbuffer == NULL) {
      pid = translateid(pid, idlist);
      sprintf(printerfilename, "%s/db/source/printer/%s.xml",
	      libdir, pid);
      printerbuffer = loadfile(printerfilename);
      if (printerbuffer == NULL) {
	printerbuffer = malloc(1024);
	make = strdup(pid);
	model = strchr(make, '-');
	if (model) {
	  t = (char *)model;
	  *t = '\0';
	  model ++;
	} else { 
	  model = "Unknown model";
	}
	t = (char *)make;
	while (*t) {
	  if (*t == '_') *t = ' ';
	  t ++;
	}
	t = (char *)model;
	while (*t) {
	  if (*t == '_') *t = ' ';
	  t ++;
	}
	sprintf((char *)printerbuffer, "<printer id=\"printer/%s\">\n <make>%s</make>\n <model>%s</model>\n <noxmlentry />\n<printer>\n", pid, make, model);
      } else {
	fprintf(stderr, 
		"WARNING: Obsolete printer ID used, using %s instead!\n",
		pid);
      }
    }
    if (debug) fprintf(stderr, "  Printer file loaded!\n");
    comboconfirmed =
      parse(&printerbuffer, pid, driver, printerfilename, NULL, 0, 
	    (const char **)defaultsettings, num_defaultsettings, &nopjl,
	    idlist, debug2);
    
    /* Read the driver file and check whether the printer is present */
    
    if (debug) fprintf(stderr, "Driver file: %s\n", driverfilename);
    driverbuffer = loadfile(driverfilename);
    if (driverbuffer == NULL) {
      if (!comboconfirmed) {
	fprintf(stderr, 
		"Driver file %s corrupted, missing, or not readable!\n",
		driverfilename);
	exit(1);
      } else {
	driverbuffer = malloc(4096);
	sprintf((char *)driverbuffer, "<driver id=\"driver/%s\">\n <name>%s</name>\n <url></url>\n <execution>\n  <filter />\n  <prototype></prototype>\n </execution>\n <printers>\n  <printer>\n   <id>printer/%s</id>\n  </printer>\n </printers>\n</driver>", driver, driver, pid);
      }
    } else {
      if (debug) fprintf(stderr, "  Driver file loaded!\n");
      comboconfirmed2 =
	parse(&driverbuffer, pid, driver, driverfilename, NULL, 1,
	      (const char **)defaultsettings, num_defaultsettings, &nopjl,
	      idlist, debug2);
      if ((!comboconfirmed) && (!comboconfirmed2)) {
	fprintf(stderr, "The printer %s is not supported by the driver %s!\n",
		pid, driver);
	exit(1);
      }
      if (debug) {
	if (nopjl) {
	  fprintf(stderr, "  Driver forbids PJL options!\n");
	} else {
	  fprintf(stderr, "  Driver allows PJL options!\n");
	}
      }
    
      /* Search the Foomatic option directory and read all xml files found
	 there. Check whether and how they apply to the given printer/driver
       combo */
    
      optiondir = opendir(optiondirname);
      if (optiondir == NULL) {
	fprintf(stderr, "Cannot read directory %s!\n", optiondirname);
	exit(1);
      }
    
      while((direntry = readdir(optiondir)) != NULL) {
	sprintf(optionfilename, "%s/db/source/opt/%s",
		libdir, direntry->d_name);
	if (debug) fprintf(stderr, "Option file: %s\n", 
			   optionfilename);
	if (strcmp((optionfilename + strlen(optionfilename) - 4), ".xml") == 0) {
	  /* Process only XML files */
	  /* Make space for a pointer to the data */
	  num_optbuffers ++;
	  optbuffers = (char **)realloc((char **)optbuffers, 
					sizeof(char *) * num_optbuffers);
	  /* load the current option's XML file */
	  optbuffers[num_optbuffers-1] = loadfile(optionfilename);
	  if (optbuffers[num_optbuffers-1] == NULL) {
	    fprintf(stderr,
		    "Option file %s corrupted, missing, or not readable!\n",
		    optionfilename);
	    exit(1);
	  }
	  if (debug) fprintf(stderr, "  Option file loaded!\n");
	  /* process it */
	  parse((char **)&(optbuffers[num_optbuffers-1]), pid, driver,
		optionfilename, NULL, 2,
		(const char **)defaultsettings, num_defaultsettings, &nopjl, 
		idlist, debug2);
	  /* If the parser discarded it (because it does not apply to our 
	     printer/driver combo) remove the space for the pointer to it */
	  if (optbuffers[num_optbuffers-1] == NULL) {
	    if (debug) fprintf(stderr, "  Option does not apply, removed!\n");
	    num_optbuffers --;
	  } else {
	    if (debug) fprintf(stderr, "  Option applies!\n");
	  }
	}
      }
      closedir(optiondir);
    }
    
    /* Output the result on STDOUT */
    if (debug) fprintf(stderr, "Putting out result!\n");
    printf("<foomatic>\n%s%s\n<options>\n", printerbuffer, driverbuffer);
    for (i = 0; i < num_optbuffers; i++) {
      printf("%s", optbuffers[i]);
    }
    printf("</options>\n</foomatic>\n");

  } else {

    /*
     * Compute XML file for the printer overview list,
     */

    /* Set file/dir names */
    
    sprintf(driverdirname, "%s/db/source/driver",
	    libdir);
    sprintf(printerdirname, "%s/db/source/printer",
	    libdir);
    
    /* Mark overview mode */
    if (overview == 2)
      if (noreadymadeppds)
	pid = "c";
      else
	pid = "C";
    else
      pid = NULL;

    /* Add a pseudo-printer to the printer list to which we assign all
       drivers without command line prototype, so we can determine
       which printer/driver combos do not provide a PPD file. */
    if (pid) {
      plistpointer = 
	(printerlist_t *)malloc(sizeof(printerlist_t));
      strcpy(plistpointer->id, "noproto");
      plistpointer->drivers = NULL;
      plistpointer->next = NULL;
      printerlist = plistpointer;
    }

    printf("<overview>\n");

    /* Search the Foomatic driver directory and read all xml files found
       there. Read out the printers which the driver supports and add them
       to the printer's driver list */
    
    driverdir = opendir(driverdirname);
    if (driverdir == NULL) {
      fprintf(stderr, "Cannot read directory %s!\n", driverdirname);
      exit(1);
    }
    
    while((direntry = readdir(driverdir)) != NULL) {
      sprintf(driverfilename, "%s/db/source/driver/%s",
	      libdir, direntry->d_name);
      if (debug) fprintf(stderr, "Driver file: %s\n", driverfilename);
      if (strcmp((driverfilename + strlen(driverfilename) - 4), ".xml") == 0) {
	/* Process only XML files */
	/* load the current driver's XML file */
	driverbuffer = loadfile(driverfilename);
	if (driverbuffer == NULL) {
	  fprintf(stderr,
		  "Driver file %s corrupted, missing, or not readable!\n",
		  driverfilename);
	  exit(1);
	}
	if (debug) fprintf(stderr, "  Driver file loaded!\n");
	/* process it */
	parse(&driverbuffer, pid, NULL, driverfilename, &printerlist, 3, 
	      (const char **)defaultsettings, num_defaultsettings, &nopjl, 
	      idlist, debug2);
	if (driverbuffer != NULL) {
	  /* put it out */
	  printf("%s\n", driverbuffer);
	  /* Delete the driver file from memory */
	  free((void *)driverbuffer);
	  driverbuffer = NULL;
	}
      }
    }
    closedir(driverdir);

    if (debug) {
      plistpointer = printerlist;
      while (plistpointer) {
	fprintf(stderr, "Printer: %s\n", plistpointer->id);
	dlistpointer = plistpointer->drivers;
	while (dlistpointer) {
	  fprintf(stderr, "   Driver: %s\n", dlistpointer->name);
	  if (dlistpointer->functionality != NULL)
	    fprintf(stderr, "    %s\n", dlistpointer->functionality);
	  dlistpointer = (driverlist_t *)(dlistpointer->next);
	}
	plistpointer = (printerlist_t *)(plistpointer->next);
      }
    }

    /* Search the Foomatic printer directory and read all xml files found
       there. Read out the printer info and build the printer entries for the
       overview with the printer/driver combo list obtained before */

    printerdir = opendir(printerdirname);
    if (printerdir == NULL) {
      fprintf(stderr, "Cannot read directory %s!\n", printerdirname);
      exit(1);
    }
    
    while((direntry = readdir(printerdir)) != NULL) {
      sprintf(printerfilename, "%s/db/source/printer/%s",
	      libdir, direntry->d_name);
      if (debug) fprintf(stderr, "Printer file: %s\n", printerfilename);
      if (strcmp((printerfilename + strlen(printerfilename) - 4), ".xml") ==
	  0) {
	/* Process only XML files */
	/* load the current printer's XML file */
	printerbuffer = loadfile(printerfilename);
	if (printerbuffer == NULL) {
	  fprintf(stderr,
		  "Printer file %s corrupted, missing, or not readable!\n",
		  printerfilename);
	  exit(1);
	}
	if (debug) fprintf(stderr, "  Printer file loaded!\n");
	/* process it */
	parse(&printerbuffer, pid, NULL, printerfilename, &printerlist, 4,
	      (const char **)defaultsettings, num_defaultsettings, &nopjl, 
	      idlist, debug2);
	/* put it out */
	printf("%s", printerbuffer);
	/* Delete the printer file from memory */
	free((void *)printerbuffer);
	printerbuffer = NULL;
      }
    }

    closedir(printerdir);

    if (debug) {
      plistpointer = printerlist;
      while (plistpointer) {
	fprintf(stderr, "Printer: %s\n", plistpointer->id);
	dlistpointer = plistpointer->drivers;
	while (dlistpointer) {
	  fprintf(stderr, "   Driver: %s\n", dlistpointer->name);
	  if (dlistpointer->functionality != NULL)
	    fprintf(stderr, "    %s\n", dlistpointer->functionality);
	  dlistpointer = (driverlist_t *)(dlistpointer->next);
	}
	plistpointer = (printerlist_t *)(plistpointer->next);
      }
    }

    if (overview < 2) {
      /* Now show all printers which are only mentioned in the lists of
	 supported prnters of the drivers and which not have a Foomatic
	 printer XML entry. This we do only for a general overview, not
         for an overview of valid PPD files ("-C" option) */
      while (printerlist) {
	if (printerlist->id && strcmp(printerlist->id, "noproto")) {
	  if (debug) fprintf(stderr, "    Printer only mentioned in driver XML files:\n      Printer ID: |%s|\n",
			     printerlist->id);
	  /*strcpy(printerlist->id, translateid(printerlist->id, idlist));*/
	  printf("  <printer>\n    <id>");
	  printf("%s", printerlist->id);
	  make = printerlist->id;
	  model = strchr(make, '-');
	  if (model) {
	    t = (char *)model;
	    *t = '\0';
	    model ++;
	  } else { 
	    model = "Unknown model";
	  }
	  t = (char *)make;
	  while (*t) {
	    if (*t == '_') *t = ' ';
	    t ++;
	  }
	  t = (char *)model;
	  while (*t) {
	    if (*t == '_') *t = ' ';
	    t ++;
	  }
	  printf("</id>\n    <make>");
	  printf("%s", make);
	  printf("</make>\n    <model>");
	  printf("%s", model);
	  printf("</model>\n    <noxmlentry />\n");
	  dlistpointer = printerlist->drivers;
	  exceptionfound = 0;
	  if (dlistpointer) {
	    printf("    <drivers>\n");
	    while (dlistpointer) {
	      if (dlistpointer->name) {
		printf("      <driver>");
		printf("%s", dlistpointer->name);
		printf("</driver>\n");
		if (dlistpointer->functionality != NULL) exceptionfound = 1;
	      }
	      dlistpointer = (driverlist_t *)(dlistpointer->next);
	    }
	    printf("    </drivers>\n");
	  }
	  if (exceptionfound) {
	    printf("    <driverfunctionalityexceptions>\n");
	    dlistpointer = printerlist->drivers;
	    while (dlistpointer) {
	      if ((dlistpointer->functionality != NULL) &&
		  (dlistpointer->name != NULL)) {
		printf("      <driverfunctionalityexception>\n");
		printf("        <driver>");
		printf("%s", dlistpointer->name);
		printf("</driver>\n");
		printf("%s", dlistpointer->functionality);
		printf("\n      </driverfunctionalityexception>\n");
	      }
	      dlistpointer = (driverlist_t *)(dlistpointer->next);
	    }
	    printf("    </driverfunctionalityexceptions>\n");
	  }
	  printf("  </printer>\n");
	}
	dlistpointer = printerlist->drivers;
	while (dlistpointer) {
	  dlistpreventry = dlistpointer;
	  dlistpointer = (driverlist_t *)(dlistpointer->next);
	  free(dlistpreventry);
	}
	plistpreventry = printerlist;
	printerlist = (printerlist_t *)(printerlist->next);
	free(plistpreventry);
      }
    }

    printf("</overview>\n");
      
  }
    
  /* Done */
  exit(0);
}

/*
 * End of "$Id$".
 */
