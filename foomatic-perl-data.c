/*
 *   Foomatic Perl Data
 *   ------------------
 *
 *   Compute the Foomatic Perl data structures out of the
 *   printer/driver combo XML file or overview XML file as generated
 *   by foomatic-combo-xml.c. Compute also a Perl data structure from
 *   printer and driver entries of the Fommatic database.
 *
 *   The Foomatic Perl data structure for the printer/driver combos is
 *   the basis for all spooler-specific config files and PPD files and
 *   also the data structure used by the filter Perl scripts.
 *
 *   The other Perl data structures are used by the Foomatic Perl library
 *   to do the database operations provided by its API.
 *
 *   This program is based on the libxml2 and this is a retianol way to
 *   make a C data structure from a single XML file. This structure is then
 *   converted to Perl
 *
 *   Copyright 2001 by Till Kamppeter
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

  Compilable with (depending on installed packages):

  gcc `xml2-config --cflags` -lxml2 -o foomatic-perl-data \
      foomatic-perl-data.c

  or

  gcc `xml-config --cflags` -lxml -o foomatic-perl-data \
      foomatic-perl-data.c

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * This program should compile and run indifferently with libxml-1.8.8 +
 * and libxml2-2.1.0 +
 * Check the COMPAT comments below
 */

/*
 * COMPAT using "xml-config --cflags" to get the include path this will
 * work with both (on distros with both xml1 and xml2 "xml2-config". 
 */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define DEBUG(x) printf(x)

/*
 * an xmlChar * is really an UTF8 encoded char string (0 terminated)
 */

/*
 * Records for the unprintable margins data
 */

typedef struct marginRecord {
  xmlChar *pagesize;
  xmlChar *unit;
  xmlChar *absolute;
  xmlChar *left;
  xmlChar *right;
  xmlChar *top;
  xmlChar *bottom;
} marginRecord, *marginRecordPtr;
  
typedef struct margins {
  int     num_marginRecords;
  marginRecordPtr *marginRecords;
} margins, *marginsPtr;

/* For driver list in printer XML, for ready-made PPDs or user-contributed
   printer entry */
typedef struct printerDrvEntry {
  xmlChar *name;
  xmlChar *comment;
  xmlChar *ppd;
  /* functionality exceptions */
  xmlChar *excmaxresx;
  xmlChar *excmaxresy;
  xmlChar *exccolor;
  xmlChar *exctext;
  xmlChar *exclineart;
  xmlChar *excgraphics;
  xmlChar *excphoto;
  xmlChar *excload;
  xmlChar *excspeed;
} printerDrvEntry, *printerDrvEntryPtr;

/*
 * Records for the data of a printer/driver combo
 */

typedef struct choice {
  xmlChar *value;
  xmlChar *comment;
  xmlChar *idx;
  xmlChar *driverval;
} choice, *choicePtr;

typedef struct arg {
  xmlChar *name;
  xmlChar *name_false;
  xmlChar *comment;
  xmlChar *idx;
  xmlChar *option_type;
  xmlChar *style;
  xmlChar *substyle;
  xmlChar *spot;
  xmlChar *order;
  xmlChar *section;
  xmlChar *grouppath;
  xmlChar *proto;
  xmlChar *required;
  xmlChar *min_value;
  xmlChar *max_value;
  xmlChar *max_length;
  xmlChar *allowed_chars;
  xmlChar *allowed_regexp;
  xmlChar *default_value;
  /* Choices for enumerated options */
  int     num_choices;
  choicePtr *choices;
} arg, *argPtr;

typedef struct comboData {
  /* Printer */
  xmlChar *id;
  xmlChar *make;
  xmlChar *model;
  xmlChar *pcmodel;
  xmlChar *ppdurl;
  /* Printer properties */
  xmlChar *color;
  xmlChar *ascii;
  xmlChar *pjl;
  xmlChar *printerppdentry;
  marginsPtr printermargins;
  /* Printer auto-detection */
  xmlChar *general_ieee;
  xmlChar *general_mfg;
  xmlChar *general_mdl;
  xmlChar *general_des;
  xmlChar *general_cmd;
  xmlChar *par_ieee;
  xmlChar *par_mfg;
  xmlChar *par_mdl;
  xmlChar *par_des;
  xmlChar *par_cmd;
  xmlChar *usb_ieee;
  xmlChar *usb_mfg;
  xmlChar *usb_mdl;
  xmlChar *usb_des;
  xmlChar *usb_cmd;
  xmlChar *snmp_ieee;
  xmlChar *snmp_mfg;
  xmlChar *snmp_mdl;
  xmlChar *snmp_des;
  xmlChar *snmp_cmd;
  xmlChar *recdriver;
  /* Driver list in printer XML file (for ready-made PPD
     links) */
  int     num_drivers;
  printerDrvEntryPtr  *drivers;
  /* Driver */
  xmlChar *driver;
  xmlChar *driver_group;
  xmlChar *pcdriver;
  xmlChar *driver_type;
  xmlChar *driver_comment;
  xmlChar *url;
  xmlChar *driver_obsolete;
  xmlChar *supplier;
  xmlChar *manufacturersupplied;
  xmlChar *license;
  xmlChar *licensetext;
  xmlChar *origlicensetext;
  xmlChar *licenselink;
  xmlChar *origlicenselink;
  xmlChar *free;
  xmlChar *patents;
  int num_supportcontacts;
  xmlChar **supportcontacts;
  xmlChar **supportcontacturls;
  xmlChar **supportcontactlevels;
  xmlChar *shortdescription;
  xmlChar *locales;
  int num_packages;
  xmlChar **packageurls;
  xmlChar **packagescopes;
  xmlChar **packagefingerprints;
  xmlChar *drvmaxresx;
  xmlChar *drvmaxresy;
  xmlChar *drvcolor;
  xmlChar *text;
  xmlChar *lineart;
  xmlChar *graphics;
  xmlChar *photo;
  xmlChar *load;
  xmlChar *speed;
  xmlChar *excmaxresx;
  xmlChar *excmaxresy;
  xmlChar *exccolor;
  xmlChar *exctext;
  xmlChar *exclineart;
  xmlChar *excgraphics;
  xmlChar *excphoto;
  xmlChar *excload;
  xmlChar *excspeed;
  int num_requires;
  xmlChar **requires;
  xmlChar **requiresversion;
  xmlChar *cmd;
  xmlChar *cmd_pdf;
  xmlChar *nopjl;
  xmlChar *nopageaccounting;
  xmlChar *driverppdentry;
  xmlChar *comboppdentry;
  marginsPtr drivermargins;
  marginsPtr combomargins;
  /* Driver options */
  int     num_args;
  argPtr  *args;
  xmlChar *maxspot;
} comboData, *comboDataPtr;

/*
 * Record for a Foomatic printer entry
 */

typedef struct printerLanguage {
  xmlChar *name;
  xmlChar *level;
} printerLanguage, *printerLanguagePtr;

typedef struct printerEntry {
  xmlChar *id;
  xmlChar *make;
  xmlChar *model;
  /* Printer properties */
  xmlChar *printer_type;
  xmlChar *color;
  xmlChar *maxxres;
  xmlChar *maxyres;
  xmlChar *refill;
  xmlChar *ascii;
  xmlChar *pjl;
  xmlChar *printerppdentry;
  marginsPtr printermargins;
  /* Printer auto-detection */
  xmlChar *general_ieee;
  xmlChar *general_mfg;
  xmlChar *general_mdl;
  xmlChar *general_des;
  xmlChar *general_cmd;
  xmlChar *par_ieee;
  xmlChar *par_mfg;
  xmlChar *par_mdl;
  xmlChar *par_des;
  xmlChar *par_cmd;
  xmlChar *usb_ieee;
  xmlChar *usb_mfg;
  xmlChar *usb_mdl;
  xmlChar *usb_des;
  xmlChar *usb_cmd;
  xmlChar *snmp_ieee;
  xmlChar *snmp_mfg;
  xmlChar *snmp_mdl;
  xmlChar *snmp_des;
  xmlChar *snmp_cmd;
  /* Misc items */
  xmlChar *functionality;
  xmlChar *driver;
  xmlChar *unverified;
  xmlChar *noxmlentry;
  xmlChar *url;
  xmlChar *contriburl;
  xmlChar *ppdurl;
  xmlChar *comment;
  /* Page Description Languages */
  int     num_languages;
  printerLanguagePtr  *languages;
  /* Drivers (for user-contributed printer entries and for ready-made PPD
     links) */
  int     num_drivers;
  printerDrvEntryPtr  *drivers;
} printerEntry, *printerEntryPtr;
  
/*
 * Records for a Foomatic driver entry
 */

typedef struct drvPrnEntry {
  xmlChar *id;
  xmlChar *comment;
  /* functionality exceptions */
  xmlChar *excmaxresx;
  xmlChar *excmaxresy;
  xmlChar *exccolor;
  xmlChar *exctext;
  xmlChar *exclineart;
  xmlChar *excgraphics;
  xmlChar *excphoto;
  xmlChar *excload;
  xmlChar *excspeed;
} drvPrnEntry, *drvPrnEntryPtr;

typedef struct driverEntry {
  xmlChar *id;
  xmlChar *name;
  xmlChar *group;
  xmlChar *url;
  xmlChar *driver_obsolete;
  xmlChar *supplier;
  xmlChar *manufacturersupplied;
  xmlChar *license;
  xmlChar *licensetext;
  xmlChar *origlicensetext;
  xmlChar *licenselink;
  xmlChar *origlicenselink;
  xmlChar *free;
  xmlChar *patents;
  int num_supportcontacts;
  xmlChar **supportcontacts;
  xmlChar **supportcontacturls;
  xmlChar **supportcontactlevels;
  xmlChar *shortdescription;
  xmlChar *locales;
  int num_packages;
  xmlChar **packageurls;
  xmlChar **packagescopes;
  xmlChar **packagefingerprints;
  xmlChar *maxresx;
  xmlChar *maxresy;
  xmlChar *color;
  xmlChar *text;
  xmlChar *lineart;
  xmlChar *graphics;
  xmlChar *photo;
  xmlChar *load;
  xmlChar *speed;
  int num_requires;
  xmlChar **requires;
  xmlChar **requiresversion;
  xmlChar *driver_type;
  xmlChar *cmd;
  xmlChar *cmd_pdf;
  xmlChar *driverppdentry;
  marginsPtr drivermargins;
  xmlChar *comment;
  int     num_printers;
  drvPrnEntryPtr *printers;
} driverEntry, *driverEntryPtr;

/*
 * Records for the data of the overview
 */

typedef struct ppdFile {
  xmlChar *driver;
  xmlChar *filename;
} ppdFile, *ppdFilePtr;

typedef struct overviewPrinter {
  /* General info */
  xmlChar *id;
  xmlChar *make;
  xmlChar *model;
  xmlChar *functionality;
  xmlChar *unverified;
  xmlChar *noxmlentry;
  /* Printer auto-detection */
  xmlChar *general_ieee;
  xmlChar *general_mfg;
  xmlChar *general_mdl;
  xmlChar *general_des;
  xmlChar *general_cmd;
  xmlChar *par_ieee;
  xmlChar *par_mfg;
  xmlChar *par_mdl;
  xmlChar *par_des;
  xmlChar *par_cmd;
  xmlChar *usb_ieee;
  xmlChar *usb_mfg;
  xmlChar *usb_mdl;
  xmlChar *usb_des;
  xmlChar *usb_cmd;
  xmlChar *snmp_ieee;
  xmlChar *snmp_mfg;
  xmlChar *snmp_mdl;
  xmlChar *snmp_des;
  xmlChar *snmp_cmd;
  /* Drivers */
  xmlChar *driver;
  int     num_drivers;
  printerDrvEntryPtr *drivers;
  /* PPD files */
  int     num_ppdfiles;
  ppdFilePtr *ppdfiles;
} overviewPrinter, *overviewPrinterPtr;
  
typedef struct overview {
  int     num_overviewDrivers;
  driverEntryPtr *overviewDrivers;
  int     num_overviewPrinters;
  overviewPrinterPtr *overviewPrinters;
} overview, *overviewPtr;

/*
 * Function to quote "'" and "\" in a string
 */

static xmlChar* /* O - String with quoted "'" */
perlquote(xmlChar *str) { /* I - Original string */
  xmlChar *dest, *s;
  int offset = 0;

  if (str == NULL) return NULL;
  dest = xmlStrdup(str);
  while ((s = (xmlChar *)xmlStrchr((const xmlChar *)(dest + offset),
				   (xmlChar)'\'')) ||
	 (s = (xmlChar *)xmlStrchr((const xmlChar *)(dest + offset),
				   (xmlChar)'\\'))) {
    offset = s - dest;
    dest = (xmlChar *)realloc((xmlChar *)dest, 
			      sizeof(xmlChar) * (xmlStrlen(dest) + 2));
    s = dest + offset;
    memmove(s + 1, s, xmlStrlen(dest) + 1 - offset);
    *s = '\\';
    offset += 2;
  }
  return(dest);
}

/*
 * Functions to read out localized text, choosing the translation into the
 * desired language. Reads also simple text without language tags, for
 * backward compatibility for the case that a not yet localized database
 * field gets localized. The second function is especially for license text.
 * It returns always the English (original) version in addition and allows
 * links to text files as alternative to the text itself, as often license text
 * has to be in its own file.
 */

static void
getLocalizedText(xmlDocPtr doc,   /* I - The whole data tree */
		 xmlNodePtr node, /* I - Node of XML tree to work on */
		 xmlChar **ret,   /* O - Text which was selected by the
				         language */
		 xmlChar *language, /* I - User language */
		 int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */

  cur1 = node->xmlChildrenNode;
  if (xmlStrcasecmp(cur1->name, "C") && xmlStrcasecmp(cur1->name, "POSIX")) {
    while (cur1 != NULL) {
      /* Exact match of locale ID */
      if ((!xmlStrcasecmp(cur1->name, language))) {
	*ret = xmlStrdup
	  (perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1)));
	if (debug)
	  fprintf
	    (stderr,
	     "    Localized text with full locale ID matched (%s):\n\n%s\n\n",
	     language, *ret);
	return;
      }
      cur1 = cur1->next;
    }
    cur1 = node->xmlChildrenNode;
    while (cur1 != NULL) {
      /* Fall back to match only the two-character language code */
      if ((!xmlStrncasecmp(cur1->name, language, 2))) {
	*ret = xmlStrdup
	  (perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1)));
	if (debug)
	  fprintf
	    (stderr,
	     "    Localized text with only language ID matched (%s):\n\n%s\n\n",
	     language, *ret);
	return;
      }
      cur1 = cur1->next;
    }
    cur1 = node->xmlChildrenNode;
  }
  while (cur1 != NULL) {
    /* Fall back to English */
    if ((!xmlStrncasecmp(cur1->name, (const xmlChar *) "en", 2))) {
      *ret = xmlStrdup
	(perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1)));
      if (debug)
	fprintf(stderr, "    English text:\n\n%s\n\n", *ret);
      return;
    }
    cur1 = cur1->next;
  }
  cur1 = node->xmlChildrenNode;
  /* Fall back to non-localized text (allows backward compatibility if
     deciding on localizing a database item later */
  *ret = xmlStrdup(perlquote(xmlNodeListGetString(doc, cur1, 1)));
  if (debug)
    fprintf(stderr, "    Non-localized text:\n\n%s\n\n", *ret);
}

static void
getLocalizedLicenseText(xmlDocPtr doc,   /* I - The whole data tree */
	 xmlNodePtr node, /* I - Node of XML tree to work on */
		 xmlChar **text,   /* O - Text which was selected by the
				         language */
		 xmlChar **origtext, /* O - Original text in English */
		 xmlChar **link,   /* O - Link which was selected by the
				         language */
		 xmlChar **origlink, /* O - Link to original text in English */
		 xmlChar *language, /* I - User language */
		 int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */
  int            localizedtextfound = 0;

  cur1 = node->xmlChildrenNode;
  if (xmlStrcasecmp(cur1->name, "C") && xmlStrcasecmp(cur1->name, "POSIX")) {
    while (cur1 != NULL) {
      /* Exact match of locale ID */
      if ((!xmlStrcasecmp(cur1->name, language))) {
	*link = xmlGetProp(cur1, (const xmlChar *) "url");
	if (*link == NULL) {
	  *text = xmlStrdup
	    (perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1)));
	  if (debug)
	    fprintf
	      (stderr,
	       "    Localized text with full locale ID matched (%s):\n\n%s\n\n",
	       language, *text);
	} else {
	  if (debug)
	    fprintf
	      (stderr,
	       "    Link to localized text with full locale ID matched (%s):\n\n%s\n\n",
	       language, *link);
	}
	localizedtextfound = 1;
	break;
      }
      cur1 = cur1->next;
    }
    if (localizedtextfound == 0) {
      cur1 = node->xmlChildrenNode;
      while (cur1 != NULL) {
	/* Fall back to match only the two-character language code */
	if ((!xmlStrncasecmp(cur1->name, language, 2))) {
	  *link = xmlGetProp(cur1, (const xmlChar *) "url");
	  if (*link == NULL) {
	    *text = xmlStrdup
	      (perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1)));
	    if (debug)
	      fprintf
		(stderr,
		 "    Localized text with only language ID matched (%s):\n\n%s\n\n",
		 language, *text);
	  } else {
	    if (debug)
	      fprintf
		(stderr,
		 "    Link to localized text with only language ID matched (%s):\n\n%s\n\n",
		 language, *link);
	  }
	  localizedtextfound = 1;
	  break;
	}
	cur1 = cur1->next;
      }
    }
  }
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    /* Fall back to English and/or extract original, English text/link */
    if ((!xmlStrncasecmp(cur1->name, (const xmlChar *) "en", 2))) {
      *origlink = xmlGetProp(cur1, (const xmlChar *) "url");
      if (*origlink == NULL) {
	*origtext = xmlStrdup
	  (perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1)));
	if (debug)
	  fprintf(stderr, "    Original English text:\n\n%s\n\n", *origtext);
	if (localizedtextfound == 0) {
	  *text = *origtext;
	  if (debug)
	    fprintf(stderr,
		    "      No localization, using original English text\n");
	}
      } else {
	if (debug)
	  fprintf(stderr, "    Link to original English text:\n\n%s\n\n",
		  *origlink);
	if (localizedtextfound == 0) {
	  *link = *origlink;
	  if (debug)
	    fprintf(stderr,
		    "      No localization, using original English link\n");
	}
      }
      return;
    }
    cur1 = cur1->next;
  }
  cur1 = node->xmlChildrenNode;
  /* Fall back to non-localized text (allows backward compatibility if
     deciding on localizing a database item later */
  *link = xmlGetProp(cur1, (const xmlChar *) "url");
  if (*link == NULL) {
    *text = xmlStrdup(perlquote(xmlNodeListGetString(doc, cur1, 1)));
    *origtext = *text;
    if (debug)
      fprintf(stderr, "    Non-localized text:\n\n%s\n\n", *text);
  } else {
    *origlink = *link;
    if (debug)
      fprintf(stderr, "    Link to non-localized text:\n\n%s\n\n", *link);
  }
}

/*
 * Functions to fill in the unprintable margin data structure with the
 * data parsed from the XML input
 */

static void
parseMarginEntry(xmlDocPtr doc,   /* I - The whole combo data tree */
		 xmlNodePtr node, /* I - Node of XML tree to work on */
		 int entrytype,   /* I - 0: General, 1: Page size 
				     exzception */
		 marginsPtr ret,  /* O - C data structure of Foomatic
					  overview */
		 xmlChar *language, /* I - User language */
		 int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */
  xmlChar        *pagesize;
  marginRecordPtr marginRec;

  /* Allocate memory for the margin record */
  ret->num_marginRecords ++;
  ret->marginRecords =
    (marginRecordPtr *)realloc
    ((marginRecordPtr *)(ret->marginRecords), 
     sizeof(marginRecordPtr) * ret->num_marginRecords);
  marginRec = (marginRecordPtr) malloc(sizeof(marginRecord));
  if (marginRec == NULL) {
    fprintf(stderr,"Out of memory!\n");
    xmlFreeDoc(doc);
    exit(1);
  }
  ret->marginRecords[ret->num_marginRecords-1] = marginRec;
  memset(marginRec, 0, sizeof(marginRecord));

  /* Initialization of entries */
  marginRec->pagesize = NULL;
  marginRec->unit = NULL;
  marginRec->absolute = NULL;
  marginRec->left = NULL;
  marginRec->right = NULL;
  marginRec->top = NULL;
  marginRec->bottom = NULL;

  /* Get page size */
  if (entrytype > 0) {
    pagesize = xmlGetProp(node, (const xmlChar *) "PageSize");
    if (pagesize != NULL) {
      marginRec->pagesize = perlquote(pagesize);
      if (debug) fprintf(stderr, "    Margins for page size %s\n", 
			 marginRec->pagesize);
    } else {
      fprintf(stderr,"Page size missing!\n");
      xmlFreeDoc(doc);
      exit(1);
    }
  } else {
    if (debug) fprintf(stderr, "    General Margins\n");
  }

  /* Go through subnodes */
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    if ((!xmlStrcmp(cur1->name, (const xmlChar *) "unit"))) {
      marginRec->unit = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "      Unit: %s\n", marginRec->unit);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "absolute"))) {
      marginRec->absolute = (xmlChar *)"1";
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "      Absolute values\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "relative"))) {
      marginRec->absolute = (xmlChar *)"0";
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "      Relative values\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "left"))) {
      marginRec->left = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "      Left margin: %s\n",
			 marginRec->left);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "right"))) {
      marginRec->right = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "      Right margin: %s\n",
			 marginRec->right);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "top"))) {
      marginRec->top = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "      Top margin: %s\n",
			 marginRec->top);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "bottom"))) {
      marginRec->bottom = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "      Bottom margin: %s\n",
			 marginRec->bottom);
    }
    cur1 = cur1->next;
  }
}

static void
parseMargins(xmlDocPtr doc,   /* I - The whole combo data tree */
	     xmlNodePtr node, /* I - Node of XML tree to work on */
	     marginsPtr *ret, /* O - C data structure of Foomatic
				 overview */
	     xmlChar *language, /* I - User language */
	     int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */

  /* Allocate memory for the margins data structure */
  *ret = (marginsPtr) malloc(sizeof(margins));
  if (*ret == NULL) {
    fprintf(stderr,"Out of memory!\n");
    xmlFreeDoc(doc);
    exit(1);
  }
  memset(*ret, 0, sizeof(margins));

  /* Initialization of entries */
  (*ret)->num_marginRecords = 0;
  (*ret)->marginRecords = NULL;

  if (debug) fprintf(stderr, "  Unprintable margins\n");

  /* Go through subnodes */
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    if ((!xmlStrcmp(cur1->name, (const xmlChar *) "general"))) {
      parseMarginEntry(doc, cur1, 0, *ret, language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "exception"))) {
      parseMarginEntry(doc, cur1, 1, *ret, language, debug);
    }
    cur1 = cur1->next;
  }
}

/*
 * Function to fill in the Foomatic overview data structure with the
 * data parsed from the XML input
 */

static void
parseOverviewPrinter(xmlDocPtr doc, /* I - The whole combo data tree */
		     xmlNodePtr node, /* I - Node of XML tree to work on */
		     overviewPtr ret, /* O - C data structure of Foomatic
					 overview */
		     xmlChar *language, /* I - User language */
		     int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */
  xmlNodePtr     cur2;  /* Another XML node pointer */
  xmlNodePtr     cur3;  /* Another XML node pointer */
  xmlNodePtr     cur4;  /* Another XML node pointer */
  xmlChar        *id;  /* Full printer ID, with "printer/" */
  xmlChar        *charset;
  xmlChar        *drivername;
  overviewPrinterPtr printer;
  printerDrvEntryPtr dentry; /* An entry for a driver supporting this
				printer */
  ppdFilePtr     ppd;
  int            driverfound;
  int            i, j;

  /* Allocate memory for the printer */
  ret->num_overviewPrinters ++;
  ret->overviewPrinters =
    (overviewPrinterPtr *)realloc
    ((overviewPrinterPtr *)(ret->overviewPrinters), 
     sizeof(overviewPrinterPtr) * ret->num_overviewPrinters);
  printer = (overviewPrinterPtr) malloc(sizeof(overviewPrinter));
  if (printer == NULL) {
    fprintf(stderr,"Out of memory!\n");
    xmlFreeDoc(doc);
    exit(1);
  }
  ret->overviewPrinters[ret->num_overviewPrinters-1] = printer;
  memset(printer, 0, sizeof(overviewPrinter));

  /* Initialization of entries */
  printer->id = NULL;
  printer->make = NULL;
  printer->model = NULL;
  printer->functionality = (xmlChar *)"X";
  printer->unverified = NULL;
  printer->noxmlentry = NULL;
  printer->general_ieee = NULL;
  printer->general_mfg = NULL;
  printer->general_mdl = NULL;
  printer->general_des = NULL;
  printer->general_cmd = NULL;
  printer->par_ieee = NULL;
  printer->par_mfg = NULL;
  printer->par_mdl = NULL;
  printer->par_des = NULL;
  printer->par_cmd = NULL;
  printer->usb_ieee = NULL;
  printer->usb_mfg = NULL;
  printer->usb_mdl = NULL;
  printer->usb_des = NULL;
  printer->usb_cmd = NULL;
  printer->snmp_ieee = NULL;
  printer->snmp_mfg = NULL;
  printer->snmp_mdl = NULL;
  printer->snmp_des = NULL;
  printer->snmp_cmd = NULL;
  printer->driver = NULL;
  printer->num_drivers = 0;
  printer->drivers = NULL;
  printer->num_ppdfiles = 0;
  printer->ppdfiles = NULL;

  /* Go through subnodes */
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    if ((!xmlStrcmp(cur1->name, (const xmlChar *) "id"))) {
      printer->id = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer ID: %s\n", printer->id);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "make"))) {
      printer->make = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer Manufacturer: %s\n", 
			 printer->make);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "model"))) {
      printer->model = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer Model: %s\n", printer->model);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "functionality"))) {
      printer->functionality = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer Functionality: %s\n",
			 printer->functionality);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "unverified"))) {
      printer->unverified = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  Printer entry is unverified\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "noxmlentry"))) {
      printer->noxmlentry = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  Printer XML entry does not exist in the database\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "driver"))) {
      printer->driver = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Recommended driver: %s\n",
			 printer->driver);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "drivers"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "driver"))) {
	  drivername =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  driverfound = 0;
	  for (i = 0; i < printer->num_drivers; i++) {
	    dentry = printer->drivers[i];
	    if ((dentry->name != NULL) &&
		(!xmlStrcmp(drivername, dentry->name))) {
	      driverfound = 1;
	      break;
	    }
	  }
	  if (!driverfound) {
	    printer->num_drivers ++;
	    printer->drivers =
	      (printerDrvEntryPtr *)
	      realloc((printerDrvEntryPtr *)printer->drivers, 
		      sizeof(printerDrvEntryPtr) * 
		      printer->num_drivers);
	    dentry = (printerDrvEntryPtr) malloc(sizeof(printerDrvEntry));
	    if (dentry == NULL) {
	      fprintf(stderr,"Out of memory!\n");
	      xmlFreeDoc(doc);
	      exit(1);
	    }
	    printer->drivers[printer->num_drivers-1] = dentry;
	    memset(dentry, 0, sizeof(printerDrvEntry));
	    dentry->name = NULL;
	    dentry->comment = NULL;
	    dentry->ppd = NULL;
	    dentry->excmaxresx = NULL;
	    dentry->excmaxresy = NULL;
	    dentry->exccolor = NULL;
	    dentry->exctext = NULL;
	    dentry->exclineart = NULL;
	    dentry->excgraphics = NULL;
	    dentry->excphoto = NULL;
	    dentry->excload = NULL;
	    dentry->excspeed = NULL;
	  }
	  dentry->name = drivername;
	  if (debug) fprintf(stderr, "  Printer works with: %s\n",
			     dentry->name);
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "driverfunctionalityexceptions"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "driverfunctionalityexception"))) {
	  drivername = NULL;
	  dentry = NULL;
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "driver"))) {
	      drivername =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1));
	      if (debug)
		fprintf(stderr, "  Functionality exceptions for driver: %s\n",
			drivername);
	      if ((dentry != NULL) && (dentry->name == NULL)) {
		dentry->name = drivername;
		/* Check whether there is already an entry for this driver
		   and delete this old entry */
		for (i = 0; i < printer->num_drivers - 1; i++) {
		  if ((printer->drivers[i]->name != NULL) &&
		      (!xmlStrcmp(drivername, printer->drivers[i]->name))) {
		    for (j = i + 1; j < printer->num_drivers; j++)
		      printer->drivers[j - 1] = printer->drivers[j];
		    printer->num_drivers --;
		    printer->drivers =
		      (printerDrvEntryPtr *)
		      realloc((printerDrvEntryPtr *)printer->drivers, 
			      sizeof(printerDrvEntryPtr) * 
			      printer->num_drivers);
		    break;
		  }
		}
	      }
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "functionality"))) {
	      driverfound = 0;
	      dentry = NULL;
	      if (drivername != NULL) {
		for (i = 0; i < printer->num_drivers; i++) {
		  dentry = printer->drivers[i];
		  if ((dentry->name != NULL) &&
		      (!xmlStrcmp(drivername, dentry->name))) {
		    driverfound = 1;
		    break;
		  }
		}
	      }
	      if (!driverfound) {
		printer->num_drivers ++;
		printer->drivers =
		  (printerDrvEntryPtr *)
		  realloc((printerDrvEntryPtr *)printer->drivers, 
			  sizeof(printerDrvEntryPtr) * 
			  printer->num_drivers);
		dentry = (printerDrvEntryPtr) malloc(sizeof(printerDrvEntry));
		if (dentry == NULL) {
		  fprintf(stderr,"Out of memory!\n");
		  xmlFreeDoc(doc);
		  exit(1);
		}
		printer->drivers[printer->num_drivers-1] = dentry;
		memset(dentry, 0, sizeof(printerDrvEntry));
		dentry->name = NULL;
		dentry->comment = NULL;
		dentry->ppd = NULL;
		dentry->excmaxresx = NULL;
		dentry->excmaxresy = NULL;
		dentry->exccolor = NULL;
		dentry->exctext = NULL;
		dentry->exclineart = NULL;
		dentry->excgraphics = NULL;
		dentry->excphoto = NULL;
		dentry->excload = NULL;
		dentry->excspeed = NULL;
	      }
	      dentry->name = drivername;
	      if (drivername != NULL) {
		if (debug)
		  fprintf(stderr, "  Functionality exceptions for driver: %s\n",
			  dentry->name);
	      }
	      cur4 = cur3->xmlChildrenNode;
	      while (cur4 != NULL) {
		if ((!xmlStrcmp(cur4->name, (const xmlChar *) "maxresx"))) {
		  dentry->excmaxresx =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Maximum X resolution: %s\n",
			    dentry->excmaxresx);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "maxresy"))) {
		  dentry->excmaxresy =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Maximum Y resolution: %s\n",
			    dentry->excmaxresy);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "monochrome"))) {
		  dentry->exccolor = (xmlChar *)"0";
		  if (debug)
		    fprintf(stderr, "    Color: %s\n",
			    dentry->exccolor);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "color"))) {
		  dentry->exccolor = (xmlChar *)"1";
		  if (debug)
		    fprintf(stderr, "    Color: %s\n",
			    dentry->exccolor);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "text"))) {
		  dentry->exctext =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Support level for text: %s\n",
			    dentry->exctext);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "lineart"))) {
		  dentry->exclineart =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Support level for line art: %s\n",
			    dentry->exclineart);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "graphics"))) {
		  dentry->excgraphics =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Support level for graphics: %s\n",
			    dentry->excgraphics);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "photo"))) {
		  dentry->excphoto =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Support level for photos: %s\n",
			    dentry->excphoto);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "load"))) {
		  dentry->excload =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Expected relative system load: %s\n",
			    dentry->excload);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "speed"))) {
		  dentry->excspeed =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Expected relative driver speed: %s\n",
			    dentry->excspeed);
		}
		cur4 = cur4->next;
	      }
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "ppds"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ppd"))) {
	  printer->num_ppdfiles ++;
	  printer->ppdfiles =
	    (ppdFilePtr *)
	    realloc((ppdFilePtr *)printer->ppdfiles, 
		    sizeof(ppdFilePtr) *
		    printer->num_ppdfiles);
	  ppd = (ppdFilePtr) malloc(sizeof(ppdFile));
	  if (ppd == NULL) {
	    fprintf(stderr,"Out of memory!\n");
	    xmlFreeDoc(doc);
	    exit(1);
	  }
	  printer->ppdfiles[printer->num_ppdfiles-1] = ppd;
	  memset(ppd, 0, sizeof(ppdFile));
	  ppd->driver = NULL;
	  ppd->filename = NULL;
	  if (debug) fprintf(stderr, "  Available ready-made PPD file:\n");
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "driver"))) {
	      ppd->driver =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1));
	      if (debug) fprintf(stderr, "    For driver: %s\n",
				 ppd->driver);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ppdfile"))) {
	      ppd->filename =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1));
	      if (debug) fprintf(stderr, "    File name: %s\n",
				 ppd->filename);
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "autodetect"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "general"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (general):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      printer->general_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n",
				 printer->general_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      printer->general_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", printer->general_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      printer->general_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", printer->general_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      printer->general_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", printer->general_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      printer->general_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", printer->general_cmd);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "parallel"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (parallel port):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      printer->par_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n",
				 printer->par_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      printer->par_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", printer->par_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      printer->par_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", printer->par_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      printer->par_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", printer->par_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      printer->par_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", printer->par_cmd);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "usb"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (USB):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      printer->usb_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n",
				 printer->usb_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      printer->usb_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", printer->usb_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      printer->usb_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", printer->usb_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      printer->usb_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", printer->usb_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      printer->usb_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", printer->usb_cmd);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "snmp"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (SNMP):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      printer->snmp_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n",
				 printer->snmp_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      printer->snmp_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", printer->snmp_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      printer->snmp_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", printer->snmp_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      printer->snmp_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", printer->snmp_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      printer->snmp_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", printer->snmp_cmd);
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    }
    cur1 = cur1->next;
  }
}

/*
 * Functions to fill in the printer/driver combo data structure with the
 * data parsed from the XML input
 */

static void
parseComboPrinter(xmlDocPtr doc, /* I - The whole combo data tree */
		  xmlNodePtr node, /* I - Node of XML tree to work on */
		  comboDataPtr ret, /* O - C data structure of Foomatic
				       combo */
		  xmlChar *language, /* I - User language */
		  int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */
  xmlNodePtr     cur2;  /* Another XML node pointer */
  xmlNodePtr     cur3;  /* Another XML node pointer */
  xmlNodePtr     cur4;  /* Another XML node pointer */
  xmlChar        *id;  /* Full printer ID, with "printer/" */
  xmlChar        *charset;
  xmlChar        *dname;  /* Name of a driver supporting this printer */
  xmlChar        *dppd;  /* Ready-made PPD supporting this printer */
  printerDrvEntryPtr dentry; /* An entry for a driver supporting this
				printer */

  /* Initialization of entries */
  ret->id = NULL;
  ret->make = NULL;
  ret->model = NULL;
  ret->pcmodel = NULL;
  ret->ppdurl = NULL;
  ret->color = (xmlChar *)"0";
  ret->pjl = (xmlChar *)"undef";
  ret->ascii = (xmlChar *)"0";
  ret->printerppdentry = NULL;
  ret->printermargins = NULL;
  ret->general_ieee = NULL;
  ret->general_mfg = NULL;
  ret->general_mdl = NULL;
  ret->general_des = NULL;
  ret->general_cmd = NULL;
  ret->par_ieee = NULL;
  ret->par_mfg = NULL;
  ret->par_mdl = NULL;
  ret->par_des = NULL;
  ret->par_cmd = NULL;
  ret->usb_ieee = NULL;
  ret->usb_mfg = NULL;
  ret->usb_mdl = NULL;
  ret->usb_des = NULL;
  ret->usb_cmd = NULL;
  ret->snmp_ieee = NULL;
  ret->snmp_mfg = NULL;
  ret->snmp_mdl = NULL;
  ret->snmp_des = NULL;
  ret->snmp_cmd = NULL;
  ret->recdriver = NULL;
  ret->num_drivers = 0;
  ret->drivers = NULL;

  /* Get printer ID */
  id = xmlGetProp(node, (const xmlChar *) "id");
  if (id == NULL) {
    fprintf(stderr, "No printer ID found\n");
    return;
  }
  ret->id = perlquote(id + 8);
  if (debug) fprintf(stderr, "  Printer ID: %s\n", ret->id);

  /* Go through subnodes */
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    if ((!xmlStrcmp(cur1->name, (const xmlChar *) "make"))) {
      ret->make = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer Manufacturer: %s\n", ret->make);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "model"))) {
      ret->model = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer Model: %s\n", ret->model);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "driver"))) {
      ret->recdriver = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Recommended driver: %s\n", 
			 ret->recdriver);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "drivers"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "driver"))) {
	  ret->num_drivers ++;
	  ret->drivers =
	    (printerDrvEntryPtr *)
	    realloc((printerDrvEntryPtr *)ret->drivers, 
		    sizeof(printerDrvEntryPtr) * 
		    ret->num_drivers);
	  dentry = (printerDrvEntryPtr) malloc(sizeof(printerDrvEntry));
	  if (dentry == NULL) {
	    fprintf(stderr,"Out of memory!\n");
	    xmlFreeDoc(doc);
	    exit(1);
	  }
	  ret->drivers[ret->num_drivers-1] = dentry;
	  memset(dentry, 0, sizeof(printerDrvEntry));
	  dentry->name = NULL;
	  dentry->comment = NULL;
	  dentry->ppd = NULL;
          dentry->excmaxresx = NULL;
	  dentry->excmaxresy = NULL;
	  dentry->exccolor = NULL;
	  dentry->exctext = NULL;
	  dentry->exclineart = NULL;
	  dentry->excgraphics = NULL;
	  dentry->excphoto = NULL;
	  dentry->excload = NULL;
	  dentry->excspeed = NULL;
	  if (debug) fprintf(stderr, "  Printer supported by drivers:\n");
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "id"))) {
	      dname =
		xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
	      dentry->name = perlquote(dname);
	      if (debug) fprintf(stderr, "    Name: %s\n",
				 dentry->name);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ppd"))) {
	      dppd =
		xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
	      dentry->ppd = perlquote(dppd);
	      if (debug) fprintf(stderr, "    Ready-made PPD: %s\n",
				 dentry->ppd);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "functionality"))) {
	      cur4 = cur3->xmlChildrenNode;
	      while (cur4 != NULL) {
		if ((!xmlStrcmp(cur4->name, (const xmlChar *) "maxresx"))) {
		  dentry->excmaxresx =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Maximum X resolution: %s\n",
			    dentry->excmaxresx);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "maxresy"))) {
		  dentry->excmaxresy =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Maximum Y resolution: %s\n",
			    dentry->excmaxresy);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "monochrome"))) {
		  dentry->exccolor = (xmlChar *)"0";
		  if (debug)
		    fprintf(stderr, "    Color: %s\n",
			    dentry->exccolor);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "color"))) {
		  dentry->exccolor = (xmlChar *)"1";
		  if (debug)
		    fprintf(stderr, "    Color: %s\n",
			    dentry->exccolor);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "text"))) {
		  dentry->exctext =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Support level for text: %s\n",
			    dentry->exctext);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "lineart"))) {
		  dentry->exclineart =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Support level for line art: %s\n",
			    dentry->exclineart);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "graphics"))) {
		  dentry->excgraphics =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Support level for graphics: %s\n",
			    dentry->excgraphics);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "photo"))) {
		  dentry->excphoto =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Support level for photos: %s\n",
			    dentry->excphoto);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "load"))) {
		  dentry->excload =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Expected relative system load: %s\n",
			    dentry->excload);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "speed"))) {
		  dentry->excspeed =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "    Expected relative driver speed: %s\n",
			    dentry->excspeed);
		}
		cur4 = cur4->next;
	      }
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "comments"))) {
	      if (debug)
		fprintf(stderr, "    Comments:\n");
	      getLocalizedText(doc, cur3, &(dentry->comment), language, debug);
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "ppdentry"))) {
      ret->printerppdentry = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Extra lines for PPD file:\n%s\n", 
			 ret->printerppdentry);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "pcmodel"))) {
      ret->pcmodel = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr,
			 "  Model part for PC filename in PPD: %s\n",
			 ret->pcmodel);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "lang"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "postscript"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ppd"))) {
	      ret->ppdurl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,
				 "  URL for the PPD for this printer: %s\n",
				 ret->ppdurl);
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "mechanism"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "color"))) {
	  ret->color = (xmlChar *)"1";
	  if (debug) fprintf(stderr, "  Color printer\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "margins"))) {
	  parseMargins(doc, cur2, &(ret->printermargins), language, debug);
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "lang"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "pjl"))) {
	  ret->pjl = (xmlChar *)"''";
	  if (debug) fprintf(stderr, "  Printer supports PJL\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "text"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "charset"))) {
	      charset =
		xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
	      if ((!xmlStrcmp(charset, (const xmlChar *) "us-ascii")) ||
		  (!xmlStrcmp(charset, (const xmlChar *) "iso-8859-1")) ||
		  (!xmlStrcmp(charset, (const xmlChar *) "iso-8859-15"))) {
		ret->ascii = (xmlChar *)"1";
		if (debug) fprintf(stderr, "  Printer prints plain text\n");
	      }
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "autodetect"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "general"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (general):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      ret->general_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n", 
				 ret->general_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      ret->general_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", ret->general_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      ret->general_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", ret->general_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      ret->general_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", ret->general_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      ret->general_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", ret->general_cmd);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "parallel"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (parallel port):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      ret->par_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n", 
				 ret->par_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      ret->par_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", ret->par_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      ret->par_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", ret->par_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      ret->par_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", ret->par_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      ret->par_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", ret->par_cmd);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "usb"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (USB):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      ret->usb_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n", 
				 ret->usb_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      ret->usb_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", ret->usb_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      ret->usb_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", ret->usb_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      ret->usb_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", ret->usb_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      ret->usb_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", ret->usb_cmd);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "snmp"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (SNMP):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      ret->snmp_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n", 
				 ret->snmp_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      ret->snmp_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", ret->snmp_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      ret->snmp_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", ret->snmp_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      ret->snmp_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", ret->snmp_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      ret->snmp_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", ret->snmp_cmd);
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    }
    cur1 = cur1->next;
  }
}

static void
parseComboDriver(xmlDocPtr doc, /* I - The whole combo data tree */
		 xmlNodePtr node, /* I - Node of XML tree to work on */
		 comboDataPtr ret, /* O - C data structure of Foomatic
				      combo */
		 xmlChar *language, /* I - User language */
		 int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */
  xmlNodePtr     cur2;  /* Another XML node pointer */
  xmlNodePtr     cur3;  /* Another XML node pointer */
  xmlNodePtr     cur4;  /* Another XML node pointer */
  xmlChar        *id;  /* Full driver ID, with "driver/" */
  xmlChar        *level, *version, *scope, *fingerprint;
  xmlChar        *url;

  /* Initialization of entries */
  ret->driver = NULL;
  ret->driver_group = NULL;
  ret->driver_type = NULL;
  ret->driver_comment = NULL;
  ret->url = NULL;
  ret->supplier = NULL;
  ret->driver_obsolete = NULL;
  ret->manufacturersupplied = NULL;
  ret->license = NULL;
  ret->licensetext = NULL;
  ret->origlicensetext = NULL;
  ret->licenselink = NULL;
  ret->origlicenselink = NULL;
  ret->free = NULL;
  ret->patents = NULL;
  ret->num_supportcontacts = 0;
  ret->supportcontacts = NULL;
  ret->supportcontacturls = NULL;
  ret->supportcontactlevels = NULL;
  ret->shortdescription = NULL;
  ret->locales = NULL;
  ret->num_packages = 0;
  ret->packageurls = NULL;
  ret->packagescopes = NULL;
  ret->packagefingerprints = NULL;
  ret->drvmaxresx = NULL;
  ret->drvmaxresy = NULL;
  ret->drvcolor = NULL;
  ret->text = NULL;
  ret->lineart = NULL;
  ret->graphics = NULL;
  ret->photo = NULL;
  ret->load = NULL;
  ret->speed = NULL;
  ret->excmaxresx = NULL;
  ret->excmaxresy = NULL;
  ret->exccolor = NULL;
  ret->exctext = NULL;
  ret->exclineart = NULL;
  ret->excgraphics = NULL;
  ret->excphoto = NULL;
  ret->excload = NULL;
  ret->excspeed = NULL;
  ret->num_requires = 0;
  ret->requires = NULL;
  ret->requiresversion = NULL;
  ret->cmd = NULL;
  ret->cmd_pdf = NULL;
  ret->nopjl = (xmlChar *)"0";
  ret->nopageaccounting = (xmlChar *)"0";
  ret->driverppdentry = NULL;
  ret->comboppdentry = NULL;
  ret->drivermargins = NULL;
  ret->combomargins = NULL;

  /* Get driver ID */
  id = xmlGetProp(node, (const xmlChar *) "id");
  if (id == NULL) {
    fprintf(stderr, "No driver ID found\n");
    return;
  }
  ret->driver = perlquote(id + 7);
  if (debug) fprintf(stderr, "  Driver ID: %s\n", ret->driver);

  /* Go through subnodes */
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    if ((!xmlStrcmp(cur1->name, (const xmlChar *) "driver"))) {
      ret->driver = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver name: %s\n", ret->driver);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "group"))) {
      ret->driver_group = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver group (for localization): %s\n", 
			 ret->driver_group);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "pcdriver"))) {
      ret->pcdriver = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver part of PC file name in PPD: %s\n",
			 ret->pcdriver);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "url"))) {
      ret->url = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver URL: %s\n", ret->url);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "obsolete"))) {
      ret->driver_obsolete = xmlGetProp(cur1, (const xmlChar *) "replace");
      if (ret->driver_obsolete == NULL) {
	if (debug) fprintf(stderr, "    No replacement driver found!\n");
	ret->driver_obsolete = (xmlChar *)"1";
      }
      if (debug) fprintf(stderr, "  Driver is obsolete: %s\n",
			 ret->driver_obsolete);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "supplier"))) {
      if (debug)
	fprintf(stderr, "  Driver supplier:\n");
      getLocalizedText(doc, cur1, &(ret->supplier), language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "manufacturersupplied"))) {
      ret->manufacturersupplied =
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if ((ret->manufacturersupplied == NULL) ||
	  (ret->manufacturersupplied[0] == '\0'))
	ret->manufacturersupplied = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  Driver supplied by manufacturer: %s\n",
			 ret->manufacturersupplied);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "thirdpartysupplied"))) {
      ret->manufacturersupplied = (xmlChar *)"0";
      if (debug) fprintf(stderr, "  Driver supplied by a third party\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "license"))) {
      if (debug)
	fprintf(stderr, "  Driver license:\n");
      getLocalizedText(doc, cur1, &(ret->license), language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "licensetext"))) {
      if (debug)
	fprintf(stderr, "  Driver license text:\n");
      getLocalizedLicenseText(doc, cur1,
			      &(ret->licensetext), &(ret->origlicensetext),
			      &(ret->licenselink), &(ret->origlicenselink),
			      language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "freesoftware"))) {
      ret->free = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  Driver is free software\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "nonfreesoftware"))) {
      ret->free = (xmlChar *)"0";
      if (debug) fprintf(stderr, "  Driver is not free software\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "patents"))) {
      ret->patents = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  There are patents applying to this driver's code\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "nopatents"))) {
      ret->patents = (xmlChar *)"0";
      if (debug) fprintf(stderr, "  There are no patents applying to this driver's code\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "supportcontacts"))) {
      cur2 = cur1->xmlChildrenNode;
      if (debug) fprintf(stderr, "  Driver support contacts:\n");
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "supportcontact"))) {
	  ret->num_supportcontacts ++;
	  ret->supportcontacts =
	    (xmlChar **)
	    realloc((xmlChar **)ret->supportcontacts, 
		    sizeof(xmlChar *) * 
		    ret->num_supportcontacts);
	  ret->supportcontacturls =
	    (xmlChar **)
	    realloc((xmlChar **)ret->supportcontacturls, 
		    sizeof(xmlChar *) * 
		    ret->num_supportcontacts);
	  ret->supportcontactlevels =
	    (xmlChar **)
	    realloc((xmlChar **)ret->supportcontactlevels, 
		    sizeof(xmlChar *) * 
		    ret->num_supportcontacts);
	  level = xmlGetProp(cur2, (const xmlChar *) "level");
	  if (level == NULL) {
	    level = (xmlChar *)"Unknown";
	  }
	  url = xmlGetProp(cur2, (const xmlChar *) "url");
	  ret->supportcontactlevels[ret->num_supportcontacts - 1] = level;
	  ret->supportcontacturls[ret->num_supportcontacts - 1] = url;
	  getLocalizedText
	    (doc, cur2,
	     &(ret->supportcontacts[ret->num_supportcontacts - 1]),
	     language, debug);
	  if (debug)
	    fprintf(stderr, "    %s (%s):\n      %s\n", 
		    ret->supportcontacts[ret->num_supportcontacts - 1],
		    ret->supportcontactlevels[ret->num_supportcontacts - 1],
		    ret->supportcontacturls[ret->num_supportcontacts - 1]);
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "shortdescription"))) {
      if (debug)
	fprintf(stderr, "  Driver short description:\n");
      getLocalizedText(doc, cur1, &(ret->shortdescription), language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "locales"))) {
      ret->locales = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver list of locales: %s\n", ret->locales);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "packages"))) {
      cur2 = cur1->xmlChildrenNode;
      if (debug) fprintf(stderr, "  Driver downloadable packages:\n");
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "package"))) {
	  ret->num_packages ++;
	  ret->packageurls =
	    (xmlChar **)
	    realloc((xmlChar **)ret->packageurls, 
		    sizeof(xmlChar *) * 
		    ret->num_packages);
	  ret->packagescopes =
	    (xmlChar **)
	    realloc((xmlChar **)ret->packagescopes, 
		    sizeof(xmlChar *) * 
		    ret->num_packages);
	  scope = xmlGetProp(cur2, (const xmlChar *) "scope");
	  ret->packagefingerprints =
	    (xmlChar **)
	    realloc((xmlChar **)ret->packagefingerprints, 
		    sizeof(xmlChar *) * 
		    ret->num_packages);
	  fingerprint = xmlGetProp(cur2, (const xmlChar *) "fingerprint");
	  ret->packageurls[ret->num_packages - 1] =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  ret->packagescopes[ret->num_packages - 1] = perlquote(scope);
	  ret->packagefingerprints[ret->num_packages - 1] = perlquote(fingerprint);
	  if (debug)
	    fprintf(stderr, "    %s (%s), key fingerprint on %s\n", 
		    ret->packageurls[ret->num_packages - 1],
		    ret->packagescopes[ret->num_packages - 1],
		    ret->packagefingerprints[ret->num_packages - 1]);
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "functionality"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "maxresx"))) {
	  ret->drvmaxresx = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Max X resolution: %s\n",
		    ret->drvmaxresx);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "maxresy"))) {
	  ret->drvmaxresy = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Max Y resolution: %s\n",
		    ret->drvmaxresy);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "color"))) {
	  ret->drvcolor = (xmlChar *)"1";
	  if (debug) fprintf(stderr, "  Driver functionality: Color\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "monochrome"))) {
	  ret->drvcolor = (xmlChar *)"0";
	  if (debug) fprintf(stderr, "  Driver functionality: Monochrome\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "text"))) {
	  ret->text = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Text support rating: %s\n",
		    ret->text);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "lineart"))) {
	  ret->lineart = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Line art support rating: %s\n",
		    ret->lineart);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "graphics"))) {
	  ret->graphics = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Graphics support rating: %s\n",
		    ret->graphics);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "photo"))) {
	  ret->photo = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Photo support rating: %s\n",
		    ret->photo);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "load"))) {
	  ret->load = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: System load rating: %s\n",
		    ret->load);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "speed"))) {
	  ret->speed = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Speed rating: %s\n",
		    ret->speed);
     	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "execution"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "requires"))) {
	  ret->num_requires ++;
	  ret->requires =
	    (xmlChar **)
	    realloc((xmlChar **)ret->requires, 
		    sizeof(xmlChar *) * 
		    ret->num_requires);
	  ret->requiresversion =
	    (xmlChar **)
	    realloc((xmlChar **)ret->requiresversion, 
		    sizeof(xmlChar *) * 
		    ret->num_requires);
	  version = xmlGetProp(cur2, (const xmlChar *) "version");
	  ret->requires[ret->num_requires - 1] =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  ret->requiresversion[ret->num_requires - 1] = perlquote(version);
	  if (debug)
	    if (!version)
	      fprintf(stderr, "  Driver requires driver: %s\n", 
		      ret->requires[ret->num_requires - 1]);
	    else
	      fprintf(stderr, "  Driver requires driver: %s (%s)\n", 
		      ret->requires[ret->num_requires - 1],
		      ret->requiresversion[ret->num_requires - 1]);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "cups"))) {
	  ret->driver_type = (xmlChar *)"C";
	  if (debug) fprintf(stderr, "  Driver type: CUPS Raster\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ijs"))) {
	  ret->driver_type = (xmlChar *)"I";
	  if (debug) fprintf(stderr, "  Driver type: IJS\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "opvp"))) {
	  ret->driver_type = (xmlChar *)"V";
	  if (debug) fprintf(stderr, "  Driver type: OpenPrinting Vector\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ghostscript"))) {
	  ret->driver_type = (xmlChar *)"G";
	  if (debug) fprintf(stderr, "  Driver type: Ghostscript built-in\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "filter"))) {
	  ret->driver_type = (xmlChar *)"F";
	  if (debug) fprintf(stderr, "  Driver type: Filter\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "uniprint"))) {
	  ret->driver_type = (xmlChar *)"U";
	  if (debug) fprintf(stderr, "  Driver type: Ghostscript Uniprint\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "postscript"))) {
	  ret->driver_type = (xmlChar *)"P";
	  if (debug) fprintf(stderr, "  Driver type: PostScript\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "nopjl"))) {
	  ret->nopjl = (xmlChar *)"1";
	  if (debug) fprintf(stderr, "  Driver suppresses PJL options\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "nopageaccounting"))) {
	  ret->nopageaccounting = (xmlChar *)"1";
	  if (debug) fprintf(stderr, "  Driver suppresses CUPS page accounting\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "prototype"))) {
	  ret->cmd =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Driver command line:\n\n    %s\n\n",
			     ret->cmd);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "prototype_pdf"))) {
	  ret->cmd_pdf =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Driver PDF command line:\n\n    %s\n\n",
			     ret->cmd_pdf);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ppdentry"))) {
	  ret->driverppdentry = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Extra lines for PPD file:\n%s\n", 
			     ret->driverppdentry);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "margins"))) {
	  parseMargins(doc, cur2, &(ret->drivermargins), language, debug);
     	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "comments"))) {
      if (debug)
	fprintf(stderr, "  Driver Comment:\n");
      getLocalizedText(doc, cur1, &(ret->driver_comment), language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "printers"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "printer"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ppdentry"))) {
	      ret->comboppdentry = 
		perlquote(xmlNodeListGetString(doc,
					       cur3->xmlChildrenNode, 1));
	      if (debug) 
		fprintf(stderr, "  Extra lines for PPD file:\n%s\n", 
			ret->comboppdentry);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "margins"))) {
	      parseMargins(doc, cur3, &(ret->combomargins), 
			   language, debug);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "functionality"))) {
	      cur4 = cur3->xmlChildrenNode;
	      while (cur4 != NULL) {
		if ((!xmlStrcmp(cur4->name, (const xmlChar *) "maxresx"))) {
		  ret->excmaxresx =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Combo exception: Maximum X resolution: %s\n",
			    ret->excmaxresx);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "maxresy"))) {
		  ret->excmaxresy =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Combo exception: Maximum Y resolution: %s\n",
			    ret->excmaxresy);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "monochrome"))) {
		  ret->exccolor = (xmlChar *)"0";
		  if (debug)
		    fprintf(stderr, "  Combo exception: Color: %s\n",
			    ret->exccolor);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "color"))) {
		  ret->exccolor = (xmlChar *)"1";
		  if (debug)
		    fprintf(stderr, "  Combo exception: Color: %s\n",
			    ret->exccolor);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "text"))) {
		  ret->exctext =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Combo exception: Support level for text: %s\n",
			    ret->exctext);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "lineart"))) {
		  ret->exclineart =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Combo exception: Support level for line art: %s\n",
			    ret->exclineart);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "graphics"))) {
		  ret->excgraphics =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Combo exception: Support level for graphics: %s\n",
			    ret->excgraphics);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "photo"))) {
		  ret->excphoto =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Combo exception: Support level for photos: %s\n",
			    ret->excphoto);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "load"))) {
		  ret->excload =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Combo exception: Expected relative system load: %s\n",
			    ret->excload);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "speed"))) {
		  ret->excspeed =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Combo exception: Expected relative driver speed: %s\n",
			    ret->excspeed);
		}
		cur4 = cur4->next;
	      }
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    }
    cur1 = cur1->next;
  }
}

static void
parseChoices(xmlDocPtr doc, /* I - The whole combo data tree */
	     xmlNodePtr node, /* I - Node of XML tree to work on */
	     argPtr option, /* O - C data structure of Foomatic option */
	     xmlChar *language, /* I - User language */
  	     int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */
  xmlNodePtr     cur2;  /* Another XML node pointer */
  xmlNodePtr     cur3;  /* Another XML node pointer */
  xmlChar        *id;   /* Full choice ID, with "ev/" */
  choicePtr      enum_val; /* Data structure for the choice currently
			      parsed */

  /* Initialization of entries */
  option->num_choices = 0;
  option->choices = NULL;

  /* Go through the choice nodes */
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    if ((!xmlStrcmp(cur1->name, (const xmlChar *) "enum_val"))) {

      /* Allocate memory for the option */
      option->num_choices ++;
      option->choices =
	(choicePtr *)realloc((choicePtr *)(option->choices), 
			     sizeof(choicePtr) * option->num_choices);
      enum_val = (choicePtr) malloc(sizeof(arg));
      if (enum_val == NULL) {
	fprintf(stderr,"Out of memory!\n");
	xmlFreeDoc(doc);
	exit(1);
      }
      option->choices[option->num_choices-1] = enum_val;
      memset(enum_val, 0, sizeof(choice));

      /* Initialization of entries */
      enum_val->value = NULL;
      enum_val->comment = NULL;
      enum_val->idx = NULL;
      enum_val->driverval = NULL;

      /* Get option ID */
      id = xmlGetProp(cur1, (const xmlChar *) "id");
      if (id == NULL) {
	fprintf(stderr, "No choice ID found\n");
	return;
      }
      enum_val->idx = perlquote(id);
      if (debug) fprintf(stderr, "    Choice ID: %s\n", enum_val->idx);

      /* Go through subnodes */
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ev_shortname"))) {
	  if (debug)
	    fprintf(stderr, "      Choice short name (do not translate):\n");
	  getLocalizedText(doc, cur2, &(enum_val->value), "C", debug);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ev_longname"))) {
	  if (debug)
	    fprintf(stderr, "      Choice long name:\n");
	  getLocalizedText(doc, cur2, &(enum_val->comment), language, debug);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ev_driverval"))) {
	  enum_val->driverval = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "      String to insert at %%s: %s\n",
			     enum_val->driverval);
	}
	cur2 = cur2->next;
      }
    }
    cur1 = cur1->next;
  }
}

static void
parseOptions(xmlDocPtr doc, /* I - The whole combo data tree */
	     xmlNodePtr node, /* I - Node of XML tree to work on */
	     comboDataPtr ret, /* O - C data structure of Foomatic combo */
	     xmlChar *language, /* I - User language */
	     int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */
  xmlNodePtr     cur2;  /* Another XML node pointer */
  xmlNodePtr     cur3;  /* Another XML node pointer */
  xmlChar        *id,  /* Full option ID, with "opt/" */
                 *option_type; /* Option type */
  argPtr         option;/* Data structure for the option currently parsed */

  /* Initialization of entries */
  ret->num_args = 0;
  ret->args = NULL;
  ret->maxspot = (xmlChar *)"A";

  /* Go through the option nodes */
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    if ((!xmlStrcmp(cur1->name, (const xmlChar *) "option"))) {

      /* Allocate memory for the option */
      ret->num_args ++;
      ret->args =
	(argPtr *)realloc((argPtr *)(ret->args), 
			  sizeof(argPtr) * ret->num_args);
      option = (argPtr) malloc(sizeof(arg));
      if (option == NULL) {
	fprintf(stderr,"Out of memory!\n");
	xmlFreeDoc(doc);
	exit(1);
      }
      ret->args[ret->num_args-1] = option;
      memset(option, 0, sizeof(arg));

      /* Initialization of entries */
      option->name = NULL;
      option->name_false = NULL;
      option->comment = NULL;
      option->idx = NULL;
      option->option_type = NULL;
      option->style = NULL;
      option->substyle= NULL;
      option->spot = NULL;
      option->order = NULL;
      option->section = NULL;
      option->grouppath = NULL;
      option->proto = NULL;
      option->required = NULL;
      option->min_value = NULL;
      option->max_value = NULL;
      option->max_length = NULL;
      option->allowed_chars = NULL;
      option->allowed_regexp = NULL;
      option->default_value = NULL;
      option->num_choices = 0;
      option->choices = NULL;

      /* Get option ID */
      id = xmlGetProp(cur1, (const xmlChar *) "id");
      if (id == NULL) {
	fprintf(stderr, "No option ID found\n");
	return;
      }
      option->idx = perlquote(id);
      if (debug) fprintf(stderr, "  Option ID: %s\n", option->idx);

      /* Get option type */
      option_type = xmlGetProp(cur1, (const xmlChar *) "type");
      if (option_type == NULL) {
	fprintf(stderr, "No option type found\n");
	return;
      }
      option->option_type = perlquote(option_type);
      if (debug) fprintf(stderr, "    Option type: %s\n",
			 option->option_type);

      /* Go through subnodes */
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_shortname"))) {
	  if (debug)
	    fprintf(stderr, "    Option short name (do not translate):\n");
	  getLocalizedText(doc, cur2, &(option->name), "C", debug);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_shortname_false"))) {
	  if (debug)
	    fprintf(stderr,
		    "    Option short name if false (do not translate):\n");
	  getLocalizedText(doc, cur2, &(option->name_false), "C", debug);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_longname"))) {
	  if (debug)
	    fprintf(stderr, "    Option long name:\n");
	  getLocalizedText(doc, cur2, &(option->comment), language, debug);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_execution"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_substitution"))) {
	      option->style = (xmlChar *)"C";
	      option->substyle = NULL;
	      if (debug)
		fprintf(stderr,
			"    Option style: Command line Substitution\n");
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_postscript"))) {
	      option->style = (xmlChar *)"G";
	      option->substyle = NULL;
	      if (debug)
		fprintf(stderr,
			"    Option style: PostScript code\n");
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_pjl"))) {
	      option->style = (xmlChar *)"J";
	      option->substyle = NULL;
	      if (debug)
		fprintf(stderr,
			"    Option style: PJL command\n");
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_composite"))) {
	      option->style = (xmlChar *)"X";
	      option->substyle = NULL;
	      if (debug)
		fprintf(stderr,
			"    Option style: Composite option\n");
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_forced_composite"))) {
	      option->style = (xmlChar *)"X";
	      option->substyle = (xmlChar *)"F";
	      if (debug)
		fprintf(stderr,
			"    Option style: Forced composite option\n");
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_spot"))) {
	      option->spot =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,
				 "    Command line insertion spot: %%%s\n",
				 option->spot);
	      if ((xmlStrcmp(option->spot, ret->maxspot) > 0)) {
		ret->maxspot = option->spot;
	      }
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_order"))) {
	      option->order =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,
				 "    Command line insertion order: %s\n",
				 option->order);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_section"))) {
	      option->section =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,
				 "    Section in PostScript file: %s\n",
				 option->section);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_group"))) {
	      option->grouppath =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,
				 "    Option Group: %s\n",
				 option->grouppath);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_proto"))) {
	      option->proto =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,
				 "    Code to insert: %s\n",
				 option->proto);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_required"))) {
	      option->required = (xmlChar *)"1";
	      if (debug) fprintf(stderr,
				 "    This option is required\n");
	    } 
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_min"))) {
	  option->min_value = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr,
			     "    Minimum value: %s\n",
			     option->min_value);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_max"))) {
	  option->max_value = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr,
			     "    Maximum value: %s\n",
			     option->max_value);
	} else if ((!xmlStrcmp(cur2->name,
			       (const xmlChar *) "arg_maxlength"))) {
	  option->max_length = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr,
			     "    Maximum string length: %s\n",
			     option->max_length);
	} else if ((!xmlStrcmp(cur2->name,
			       (const xmlChar *) "arg_allowedchars"))) {
	  option->allowed_chars = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr,
			     "    Allowed characters in string: %s\n",
			     option->allowed_chars);
	} else if ((!xmlStrcmp(cur2->name,
			       (const xmlChar *) "arg_allowedregexp"))) {
	  option->allowed_regexp = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr,
			     "    String must match Perl regexp: %s\n",
			     option->allowed_regexp);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_defval"))) {
	  option->default_value = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr,
			     "    Default: %s\n",
			     option->default_value);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "enum_vals"))) {
	  if (debug) fprintf(stderr, "  --> Parsing choice data\n");
	  parseChoices(doc, cur2, option, language, debug);
	}
	cur2 = cur2->next;
      }
    }
    cur1 = cur1->next;
  }
  if (debug) fprintf(stderr,
		     "  Last command line insertion spot: %%%s\n",
		     ret->maxspot);
}

/*
 * Functions to fill in the printer and driver data structures with the
 * data parsed from the XML input
 */

static void
parsePrinterEntry(xmlDocPtr doc, /* I - The whole printer data tree */
		  xmlNodePtr node, /* I - Node of XML tree to work on */
		  printerEntryPtr ret, /* O - C data structure of Foomatic
					  printer entry */
		  xmlChar *language, /* I - User language */
		  int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */
  xmlNodePtr     cur2;  /* Another XML node pointer */
  xmlNodePtr     cur3;  /* Another XML node pointer */
  xmlNodePtr     cur4;  /* Another XML node pointer */
  xmlChar        *id;  /* Full printer ID, with "printer/" */
  xmlChar        *dname;  /* Name of a driver supporting this printer */
  xmlChar        *dppd;  /* Ready-made PPD supporting this printer */
  printerLanguagePtr lentry; /* An entry for a language used by this
				printer */
  printerDrvEntryPtr dentry; /* An entry for a driver supporting this
				printer */

  /* Initialization of entries */
  ret->id = NULL;
  ret->make = NULL;
  ret->model = NULL;
  ret->printer_type = NULL;
  ret->color = (xmlChar *)"0"; 
  ret->maxxres = NULL;
  ret->maxyres = NULL;
  ret->printerppdentry = NULL;
  ret->printermargins = NULL;
  ret->refill = NULL;
  ret->ascii = NULL;
  ret->pjl = (xmlChar *)"0";
  ret->general_ieee = NULL;
  ret->general_mfg = NULL;
  ret->general_mdl = NULL;
  ret->general_des = NULL;
  ret->general_cmd = NULL;
  ret->par_ieee = NULL;
  ret->par_mfg = NULL;
  ret->par_mdl = NULL;
  ret->par_des = NULL;
  ret->par_cmd = NULL;
  ret->usb_ieee = NULL;
  ret->usb_mfg = NULL;
  ret->usb_mdl = NULL;
  ret->usb_des = NULL;
  ret->usb_cmd = NULL;
  ret->snmp_ieee = NULL;
  ret->snmp_mfg = NULL;
  ret->snmp_mdl = NULL;
  ret->snmp_des = NULL;
  ret->snmp_cmd = NULL;
  ret->functionality = (xmlChar *)"X";
  ret->driver = NULL;
  ret->unverified = (xmlChar *)"0";
  ret->noxmlentry = (xmlChar *)"0";
  ret->url = NULL;
  ret->contriburl = NULL;
  ret->comment = NULL;
  ret->ppdurl = NULL;
  ret->num_languages = 0;
  ret->languages = NULL;
  ret->num_drivers = 0;
  ret->drivers = NULL;

  /* Get printer ID */
  id = xmlGetProp(node, (const xmlChar *) "id");
  if (id == NULL) {
    fprintf(stderr, "No printer ID found\n");
    return;
  }
  ret->id = perlquote(id + 8);
  if (debug) fprintf(stderr, "  Printer ID: %s\n", ret->id);

  /* Go through subnodes */
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    if ((!xmlStrcmp(cur1->name, (const xmlChar *) "make"))) {
      ret->make = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer Manufacturer: %s\n", ret->make);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "model"))) {
      ret->model = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer Model: %s\n", ret->model);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "mechanism"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
 	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "inkjet"))) {
	  ret->printer_type = (xmlChar *)"inkjet";
	  if (debug) fprintf(stderr, "  Inkjet printer\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "laser"))) {
	  ret->printer_type = (xmlChar *)"laser";
	  if (debug) fprintf(stderr, "  Laser printer\n");
 	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "impact"))) {
	  ret->printer_type = (xmlChar *)"impact";
	  if (debug) fprintf(stderr, "  Impact printer\n");
 	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "dotmatrix"))) {
	  ret->printer_type = (xmlChar *)"dotmatrix";
	  if (debug) fprintf(stderr, "  Dot matrix printer\n");
 	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "led"))) {
	  ret->printer_type = (xmlChar *)"led";
	  if (debug) fprintf(stderr, "  LED printer\n");
 	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "sublimation"))) {
	  ret->printer_type = (xmlChar *)"sublimation";
	  if (debug) fprintf(stderr, "  Dye sublimation printer\n");
 	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "transfer"))) {
	  ret->printer_type = (xmlChar *)"transfer";
	  if (debug) fprintf(stderr, "  Thermal transfer printer\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "color"))) {
	  ret->color = (xmlChar *)"1";
	  if (debug) fprintf(stderr, "  Color printer\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "monochrome"))) {
	  ret->color = (xmlChar *)"0";
	  if (debug) fprintf(stderr, "  Monochrome printer\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "resolution"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "dpi"))) {
	      cur4 = cur3->xmlChildrenNode;
	      while (cur4 != NULL) {
		if ((!xmlStrcmp(cur4->name, (const xmlChar *) "x"))) {
		  ret->maxxres = 
		    perlquote(xmlNodeListGetString(doc,
						   cur4->xmlChildrenNode,
						   1));
		  if (debug) fprintf(stderr,
				     "  Maximum X resolution: %s\n",
				     ret->maxxres);
		  
		} else if ((!xmlStrcmp(cur4->name,(const xmlChar *) "y"))) {
		  ret->maxyres = 
		    perlquote(xmlNodeListGetString(doc,
						   cur4->xmlChildrenNode,
						   1));
		  if (debug) fprintf(stderr,
				     "  Maximum Y resolution: %s\n",
				     ret->maxyres);
		  
		}
		cur4 = cur4->next;
	      }
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "margins"))) {
	  parseMargins(doc, cur2, &(ret->printermargins), language, debug);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "consumables"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "comments"))) {
	      if (debug)
		fprintf(stderr, "  Consumables:\n");
	      getLocalizedText(doc, cur3, &(ret->refill), language, debug);
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "lang"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "pjl"))) {
	  ret->pjl = (xmlChar *)"1";
	  if (debug) fprintf(stderr, "  Printer supports PJL\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "text"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "charset"))) {
	      ret->ascii =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,
				 "  Printer prints plain text: %s\n",
				 ret->ascii);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "postscript"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ppd"))) {
	      ret->ppdurl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,
				 "  URL for the PPD for this printer: %s\n",
				 ret->ppdurl);
	    }
	    cur3 = cur3->next;
	  }
	}
	if ((xmlStrcmp(cur2->name, (const xmlChar *) "pjl")) &&
	    (xmlStrcmp(cur2->name, (const xmlChar *) "text")) &&
	    (xmlStrcmp(cur2->name, (const xmlChar *) "comment"))) {
	  ret->num_languages ++;
	  ret->languages =
	    (printerLanguagePtr *)
	    realloc((printerLanguagePtr *)ret->languages, 
		    sizeof(printerLanguagePtr) * 
		    ret->num_languages);
	  lentry = (printerLanguagePtr) malloc(sizeof(printerLanguage));
	  if (lentry == NULL) {
	    fprintf(stderr,"Out of memory!\n");
	    xmlFreeDoc(doc);
	    exit(1);
	  }
	  ret->languages[ret->num_languages-1] = lentry;
	  memset(lentry, 0, sizeof(printerLanguage));
	  lentry->name = perlquote((xmlChar *)(cur2->name));
	  lentry->level =
	    perlquote(xmlGetProp(cur2, (const xmlChar *) "level"));
	  if (lentry->level == NULL) lentry->level = (xmlChar *) "";
	  if (debug)
	    fprintf(stderr, "  Printer understands PDL: %s Level %s\n",
		    lentry->name, lentry->level);
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "autodetect"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "general"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (general):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      ret->general_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n", 
				 ret->general_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      ret->general_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", ret->general_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      ret->general_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", ret->general_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      ret->general_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", ret->general_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      ret->general_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", ret->general_cmd);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "parallel"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (parallel port):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      ret->par_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n", 
				 ret->par_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      ret->par_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", ret->par_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      ret->par_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", ret->par_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      ret->par_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", ret->par_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      ret->par_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", ret->par_cmd);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "usb"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (USB):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      ret->usb_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n", 
				 ret->usb_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      ret->usb_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", ret->usb_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      ret->usb_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", ret->usb_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      ret->usb_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", ret->usb_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      ret->usb_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", ret->usb_cmd);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "snmp"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (SNMP):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ieee1284"))) {
	      ret->snmp_ieee =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    IEEE1284: %s\n", 
				 ret->snmp_ieee);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
	      ret->snmp_mfg =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode, 
					       1));
	      if (debug) fprintf(stderr, "    MFG: %s\n", ret->snmp_mfg);

	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "model"))) {
	      ret->snmp_mdl =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    MDL: %s\n", ret->snmp_mdl);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "description"))) {
	      ret->snmp_des =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    DES: %s\n", ret->snmp_des);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "commandset"))) {
	      ret->snmp_cmd =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    CMD: %s\n", ret->snmp_cmd);
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "functionality"))) {
      ret->functionality = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer Functionality: %s\n",
			 ret->functionality);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "driver"))) {
      ret->driver = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Recommended driver: %s\n", ret->driver);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "ppdentry"))) {
      ret->printerppdentry = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Extra lines for PPD file:\n%s\n", 
			 ret->printerppdentry);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "unverified"))) {
      ret->unverified = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  Printer entry is unverified\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "noxmlentry"))) {
      ret->noxmlentry = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  Printer XML entry does not exist in the database\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "url"))) {
      ret->url = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Printer URL: %s\n", ret->url);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "contrib_url"))) {
      ret->contriburl = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Contributed URL: %s\n",
			 ret->contriburl);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "comments"))) {
      if (debug)
	fprintf(stderr, "  Comment:\n");
      getLocalizedText(doc, cur1, &(ret->comment), language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "drivers"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "driver"))) {
	  ret->num_drivers ++;
	  ret->drivers =
	    (printerDrvEntryPtr *)
	    realloc((printerDrvEntryPtr *)ret->drivers, 
		    sizeof(printerDrvEntryPtr) * 
		    ret->num_drivers);
	  dentry = (printerDrvEntryPtr) malloc(sizeof(printerDrvEntry));
	  if (dentry == NULL) {
	    fprintf(stderr,"Out of memory!\n");
	    xmlFreeDoc(doc);
	    exit(1);
	  }
	  ret->drivers[ret->num_drivers-1] = dentry;
	  memset(dentry, 0, sizeof(printerDrvEntry));
	  dentry->name = NULL;
	  dentry->comment = NULL;
	  dentry->ppd = NULL;
          dentry->excmaxresx = NULL;
	  dentry->excmaxresy = NULL;
	  dentry->exccolor = NULL;
	  dentry->exctext = NULL;
	  dentry->exclineart = NULL;
	  dentry->excgraphics = NULL;
	  dentry->excphoto = NULL;
	  dentry->excload = NULL;
	  dentry->excspeed = NULL;
	  if (debug) fprintf(stderr, "  Printer supported by drivers:\n");
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "id"))) {
	      dname =
		xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
	      dentry->name = perlquote(dname);
	      if (debug) fprintf(stderr, "    Name: %s\n",
				 dentry->name);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "ppd"))) {
	      dppd =
		xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
	      dentry->ppd = perlquote(dppd);
	      if (debug) fprintf(stderr, "    Ready-made PPD: %s\n",
				 dentry->ppd);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "comments"))) {
	      if (debug)
		fprintf(stderr, "    Comment:\n");
	      getLocalizedText(doc, cur3, &(dentry->comment), language, debug);
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    }
    cur1 = cur1->next;
  }
}

static void
parseDriverEntry(xmlDocPtr doc, /* I - The whole driver data tree */
		 xmlNodePtr node, /* I - Node of XML tree to work on */
		 driverEntryPtr ret, /* O - C data structure of Foomatic
					driver entry */
		 xmlChar *language, /* I - User language */
		 int debug) { /* I - Debug mode flag */
  xmlNodePtr     cur1;  /* XML node currently worked on */
  xmlNodePtr     cur2;  /* Another XML node pointer */
  xmlNodePtr     cur3;  /* Another XML node pointer */
  xmlNodePtr     cur4;  /* Another XML node pointer */
  xmlChar        *id;   /* Full driver ID, with "driver/" */
  drvPrnEntryPtr entry; /* An entry for a printer supported by this driver*/
  xmlChar        *url, *level, *version, *scope, *fingerprint;

  /* Initialization of entries */
  ret->id = NULL;
  ret->name = NULL;
  ret->group = NULL;
  ret->url = NULL;
  ret->driver_obsolete = NULL;
  ret->supplier = NULL;
  ret->manufacturersupplied = NULL;
  ret->license = NULL;
  ret->licensetext = NULL;
  ret->origlicensetext = NULL;
  ret->licenselink = NULL;
  ret->origlicenselink = NULL;
  ret->free = NULL;
  ret->patents = NULL;
  ret->num_supportcontacts = 0;
  ret->supportcontacts = NULL;
  ret->supportcontacturls = NULL;
  ret->supportcontactlevels = NULL;
  ret->shortdescription = NULL;
  ret->locales = NULL;
  ret->num_packages = 0;
  ret->packageurls = NULL;
  ret->packagescopes = NULL;
  ret->packagefingerprints = NULL;
  ret->maxresx = NULL;
  ret->maxresy = NULL;
  ret->color = NULL;
  ret->text = NULL;
  ret->lineart = NULL;
  ret->graphics = NULL;
  ret->photo = NULL;
  ret->load = NULL;
  ret->speed = NULL;
  ret->num_requires = 0;
  ret->requires = NULL;
  ret->requiresversion = NULL;
  ret->driver_type = NULL;
  ret->cmd = NULL;
  ret->cmd_pdf = NULL;
  ret->driverppdentry = NULL;
  ret->drivermargins = NULL;
  ret->comment = NULL;
  ret->num_printers = 0;
  ret->printers = NULL;

  /* Get driver ID */
  id = xmlGetProp(node, (const xmlChar *) "id");
  if (id == NULL) {
    fprintf(stderr, "No driver ID found\n");
    return;
  }
  ret->id = perlquote(id + 7);
  if (debug) fprintf(stderr, "  Driver ID: %s\n", ret->id);

  /* Go through subnodes */
  cur1 = node->xmlChildrenNode;
  while (cur1 != NULL) {
    if ((!xmlStrcmp(cur1->name, (const xmlChar *) "name"))) {
      ret->name = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver name: %s\n", ret->name);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "group"))) {
      ret->group = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver group (for localization): %s\n",
			 ret->group);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "url"))) {
      ret->url = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver URL: %s\n", ret->url);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "obsolete"))) {
      ret->driver_obsolete = xmlGetProp(cur1, (const xmlChar *) "replace");
      if (ret->driver_obsolete == NULL) {
	if (debug) fprintf(stderr, "    No replacement driver found!\n");
	ret->driver_obsolete = (xmlChar *)"1";
      }
      if (debug) fprintf(stderr, "  Driver is obsolete: %s\n",
			 ret->driver_obsolete);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "supplier"))) {
      if (debug)
	fprintf(stderr, "  Driver supplier:\n");
      getLocalizedText(doc, cur1, &(ret->supplier), language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "manufacturersupplied"))) {
      ret->manufacturersupplied =
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if ((ret->manufacturersupplied == NULL) ||
	  (ret->manufacturersupplied[0] == '\0'))
	ret->manufacturersupplied = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  Driver supplied by manufacturer: %s\n",
			 ret->manufacturersupplied);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "thirdpartysupplied"))) {
      ret->manufacturersupplied = (xmlChar *)"0";
      if (debug) fprintf(stderr, "  Driver supplied by a third party\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "license"))) {
      if (debug)
	fprintf(stderr, "  Driver license:\n");
      getLocalizedText(doc, cur1, &(ret->license), language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "licensetext"))) {
      if (debug)
	fprintf(stderr, "  Driver license text:\n");
      getLocalizedLicenseText(doc, cur1,
			      &(ret->licensetext), &(ret->origlicensetext),
			      &(ret->licenselink), &(ret->origlicenselink),
			      language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "freesoftware"))) {
      ret->free = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  Driver is free software\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "nonfreesoftware"))) {
      ret->free = (xmlChar *)"0";
      if (debug) fprintf(stderr, "  Driver is not free software\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "patents"))) {
      ret->patents = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  There are patents applying to this driver's code\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "nopatents"))) {
      ret->patents = (xmlChar *)"0";
      if (debug) fprintf(stderr, "  There are no patents applying to this driver's code\n");
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "supportcontacts"))) {
      cur2 = cur1->xmlChildrenNode;
      if (debug) fprintf(stderr, "  Driver support contacts:\n");
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "supportcontact"))) {
	  ret->num_supportcontacts ++;
	  ret->supportcontacts =
	    (xmlChar **)
	    realloc((xmlChar **)ret->supportcontacts, 
		    sizeof(xmlChar *) * 
		    ret->num_supportcontacts);
	  ret->supportcontacturls =
	    (xmlChar **)
	    realloc((xmlChar **)ret->supportcontacturls, 
		    sizeof(xmlChar *) * 
		    ret->num_supportcontacts);
	  ret->supportcontactlevels =
	    (xmlChar **)
	    realloc((xmlChar **)ret->supportcontactlevels, 
		    sizeof(xmlChar *) * 
		    ret->num_supportcontacts);
	  level = xmlGetProp(cur2, (const xmlChar *) "level");
	  if (level == NULL) {
	    level = (xmlChar *)"Unknown";
	  }
	  url = xmlGetProp(cur2, (const xmlChar *) "url");
	  ret->supportcontactlevels[ret->num_supportcontacts - 1] = level;
	  ret->supportcontacturls[ret->num_supportcontacts - 1] = url;
	  getLocalizedText
	    (doc, cur2,
	     &(ret->supportcontacts[ret->num_supportcontacts - 1]),
	     language, debug);
	  if (debug)
	    fprintf(stderr, "    %s (%s):\n      %s\n", 
		    ret->supportcontacts[ret->num_supportcontacts - 1],
		    ret->supportcontactlevels[ret->num_supportcontacts - 1],
		    ret->supportcontacturls[ret->num_supportcontacts - 1]);
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "shortdescription"))) {
      if (debug)
	fprintf(stderr, "  Driver short description:\n");
      getLocalizedText(doc, cur1, &(ret->shortdescription), language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "locales"))) {
      ret->locales = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver list of locales: %s\n", ret->locales);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "packages"))) {
      cur2 = cur1->xmlChildrenNode;
      if (debug) fprintf(stderr, "  Driver downloadable packages:\n");
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "package"))) {
	  ret->num_packages ++;
	  ret->packageurls =
	    (xmlChar **)
	    realloc((xmlChar **)ret->packageurls, 
		    sizeof(xmlChar *) * 
		    ret->num_packages);
	  ret->packagescopes =
	    (xmlChar **)
	    realloc((xmlChar **)ret->packagescopes, 
		    sizeof(xmlChar *) * 
		    ret->num_packages);
	  scope = xmlGetProp(cur2, (const xmlChar *) "scope");
	  ret->packagefingerprints =
	    (xmlChar **)
	    realloc((xmlChar **)ret->packagefingerprints, 
		    sizeof(xmlChar *) * 
		    ret->num_packages);
	  fingerprint = xmlGetProp(cur2, (const xmlChar *) "fingerprint");
	  ret->packageurls[ret->num_packages - 1] =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  ret->packagescopes[ret->num_packages - 1] = perlquote(scope);
	  ret->packagefingerprints[ret->num_packages - 1] = perlquote(fingerprint);
	  if (debug)
	    fprintf(stderr, "    %s (%s), key fingerprint on %s\n", 
		    ret->packageurls[ret->num_packages - 1],
		    ret->packagescopes[ret->num_packages - 1],\
		    ret->packagefingerprints[ret->num_packages - 1]);
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "functionality"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "maxresx"))) {
	  ret->maxresx = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Max X resolution: %s\n",
		    ret->maxresx);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "maxresy"))) {
	  ret->maxresy = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Max Y resolution: %s\n",
		    ret->maxresy);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "color"))) {
	  ret->color = (xmlChar *)"1";
	  if (debug) fprintf(stderr, "  Driver functionality: Color\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "monochrome"))) {
	  ret->color = (xmlChar *)"0";
	  if (debug) fprintf(stderr, "  Driver functionality: Monochrome\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "text"))) {
	  ret->text = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Text support rating: %s\n",
		    ret->text);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "lineart"))) {
	  ret->lineart = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Line art support rating: %s\n",
		    ret->lineart);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "graphics"))) {
	  ret->graphics = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Graphics support rating: %s\n",
		    ret->graphics);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "photo"))) {
	  ret->photo = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Photo support rating: %s\n",
		    ret->photo);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "load"))) {
	  ret->load = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: System load rating: %s\n",
		    ret->load);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "speed"))) {
	  ret->speed = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug)
	    fprintf(stderr, "  Driver functionality: Speed rating: %s\n",
		    ret->speed);
     	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "execution"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "requires"))) {
	  ret->num_requires ++;
	  ret->requires =
	    (xmlChar **)
	    realloc((xmlChar **)ret->requires, 
		    sizeof(xmlChar *) * 
		    ret->num_requires);
	  ret->requiresversion =
	    (xmlChar **)
	    realloc((xmlChar **)ret->requiresversion, 
		    sizeof(xmlChar *) * 
		    ret->num_requires);
	  version = xmlGetProp(cur2, (const xmlChar *) "version");
	  ret->requires[ret->num_requires - 1] =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  ret->requiresversion[ret->num_requires - 1] = perlquote(version);
	  if (debug)
	    if (!version)
	      fprintf(stderr, "  Driver requires driver: %s\n", 
		      ret->requires[ret->num_requires - 1]);
	    else
	      fprintf(stderr, "  Driver requires driver: %s (%s)\n", 
		      ret->requires[ret->num_requires - 1],
		      ret->requiresversion[ret->num_requires - 1]);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "cups"))) {
	  ret->driver_type = (xmlChar *)"C";
	  if (debug) fprintf(stderr, "  Driver type: CUPS Raster\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ijs"))) {
	  ret->driver_type = (xmlChar *)"I";
	  if (debug) fprintf(stderr, "  Driver type: IJS\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "opvp"))) {
	  ret->driver_type = (xmlChar *)"V";
	  if (debug) fprintf(stderr, "  Driver type: OpenPrinting Vector\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ghostscript"))) {
	  ret->driver_type = (xmlChar *)"G";
	  if (debug) fprintf(stderr, "  Driver type: Ghostscript built-in\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "filter"))) {
	  ret->driver_type = (xmlChar *)"F";
	  if (debug) fprintf(stderr, "  Driver type: Filter\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "uniprint"))) {
	  ret->driver_type = (xmlChar *)"U";
	  if (debug) fprintf(stderr, "  Driver type: Ghostscript Uniprint\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "postscript"))) {
	  ret->driver_type = (xmlChar *)"P";
	  if (debug) fprintf(stderr, "  Driver type: PostScript\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "prototype"))) {
	  ret->cmd =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Driver command line:\n\n    %s\n\n",
			     ret->cmd);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "prototype_pdf"))) {
	  ret->cmd_pdf =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Driver PDF command line:\n\n    %s\n\n",
			     ret->cmd_pdf);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ppdentry"))) {
	  ret->driverppdentry = 
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Extra lines for PPD file:\n%s\n", 
			     ret->driverppdentry);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "margins"))) {
	  parseMargins(doc, cur2, &(ret->drivermargins), language, debug);
     	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "comments"))) {
      if (debug)
	fprintf(stderr, "  Comment:\n");
      getLocalizedText(doc, cur1, &(ret->comment), language, debug);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "printers"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "printer"))) {
	  ret->num_printers ++;
	  ret->printers =
	    (drvPrnEntryPtr *)realloc((drvPrnEntryPtr *)ret->printers, 
				      sizeof(drvPrnEntryPtr) * 
				      ret->num_printers);
	  entry = (drvPrnEntryPtr) malloc(sizeof(drvPrnEntry));
	  if (entry == NULL) {
	    fprintf(stderr,"Out of memory!\n");
	    xmlFreeDoc(doc);
	    exit(1);
	  }
	  ret->printers[ret->num_printers-1] = entry;
	  memset(entry, 0, sizeof(drvPrnEntry));
	  entry->id = NULL;
	  entry->comment = NULL;
          entry->excmaxresx = NULL;
	  entry->excmaxresy = NULL;
	  entry->exccolor = NULL;
	  entry->exctext = NULL;
	  entry->exclineart = NULL;
	  entry->excgraphics = NULL;
	  entry->excphoto = NULL;
	  entry->excload = NULL;
	  entry->excspeed = NULL;
	  if (debug) fprintf(stderr, "  Driver supports printer:\n");
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "id"))) {
	      id =
		xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
	      entry->id = perlquote(id + 8);
	      if (debug) fprintf(stderr, "    ID: %s\n",
				 entry->id);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "comments"))) {
	      if (debug)
		fprintf(stderr, "    Comment:\n");
	      getLocalizedText(doc, cur3, &(entry->comment), language, debug);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "functionality"))) {
	      cur4 = cur3->xmlChildrenNode;
	      while (cur4 != NULL) {
		if ((!xmlStrcmp(cur4->name, (const xmlChar *) "maxresx"))) {
		  entry->excmaxresx =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Printer exception: Maximum X resolution: %s\n",
			    entry->excmaxresx);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "maxresy"))) {
		  entry->excmaxresy =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Printer exception: Maximum Y resolution: %s\n",
			    entry->excmaxresy);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "monochrome"))) {
		  entry->exccolor = (xmlChar *)"0";
		  if (debug)
		    fprintf(stderr, "  Printer exception: Color: %s\n",
			    entry->exccolor);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "color"))) {
		  entry->exccolor = (xmlChar *)"1";
		  if (debug)
		    fprintf(stderr, "  Printer exception: Color: %s\n",
			    entry->exccolor);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "text"))) {
		  entry->exctext =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Printer exception: Support level for text: %s\n",
			    entry->exctext);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "lineart"))) {
		  entry->exclineart =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Printer exception: Support level for line art: %s\n",
			    entry->exclineart);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "graphics"))) {
		  entry->excgraphics =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Printer exception: Support level for graphics: %s\n",
			    entry->excgraphics);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "photo"))) {
		  entry->excphoto =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Printer exception: Support level for photos: %s\n",
			    entry->excphoto);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "load"))) {
		  entry->excload =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Printer exception: Expected relative system load: %s\n",
			    entry->excload);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "speed"))) {
		  entry->excspeed =
		    perlquote(xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1));
		  if (debug)
		    fprintf(stderr, "  Printer exception: Expected relative driver speed: %s\n",
			    entry->excspeed);
		}
		cur4 = cur4->next;
	      }
	    }
	    cur3 = cur3->next;
	  }
	}
	cur2 = cur2->next;
      }
    }
    cur1 = cur1->next;
  }
}

static xmlDocPtr /* O - Tree with data parsed from stdin */
parseXMLFromStdin() {

  FILE *f;
  xmlDocPtr doc = NULL;

  if (stdin != NULL) {
    int res, size = 1024;
    char chars[1024];
    xmlParserCtxtPtr ctxt;
    
    res = fread(chars, 1, 4, stdin);
    if (res > 0) {
      ctxt = xmlCreatePushParserCtxt(NULL, NULL,
				     chars, res, NULL);
      while ((res = fread(chars, 1, size, stdin)) > 0) {
	xmlParseChunk(ctxt, chars, res, 0);
      }
      xmlParseChunk(ctxt, chars, 0, 1);
      doc = ctxt->myDoc;
      xmlFreeParserCtxt(ctxt);
    }
  }
  return doc;
}

static overviewPtr     /* O - C data structure of overview */
parseOverviewFile(char *filename, /* I - Input file name, NULL: stdin */
		  xmlChar *language, /* I - User language */
		  int debug) { /* I - Debug mode flag */
  xmlDocPtr      doc;  /* Output of XML parser */
  overviewPtr    ret;  /* C data structure of overview */
  xmlNodePtr     cur;  /* XML node currently worked on */
  driverEntryPtr driver;
  
  /*
   * build an XML tree from a file or stdin;
   */
  
  if (filename == NULL) {
    doc = parseXMLFromStdin();
  } else {
    doc = xmlParseFile(filename);
  }
  if (doc == NULL) return(NULL);
  
  /*
   * Check the document is of the right kind
   */
  
  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    fprintf(stderr,"Empty input document!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  
  if (xmlStrcmp(cur->name, (const xmlChar *) "overview")) {
    fprintf(stderr,"Input document is not a Foomatic overview XML file (no \"<overview>\" tag)!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  
  /*
   * Allocate the structure to be returned.
   */
  ret = (overviewPtr) malloc(sizeof(overview));
  if (ret == NULL) {
    fprintf(stderr,"Out of memory!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  memset(ret, 0, sizeof(overview));
  ret->num_overviewPrinters = 0;
  ret->overviewPrinters = NULL;

  /*
   * Now, walk through the tree.
   */

  /* On the first level we have the printer entries */

  cur = cur->xmlChildrenNode;
  while (cur) {
    if (xmlIsBlankNode(cur)) {
      cur = cur->next;
      continue;
    }
    if (!xmlStrcmp(cur->name, (const xmlChar *) "driver")) {
      ret->num_overviewDrivers ++;
      ret->overviewDrivers =
	(driverEntryPtr *)realloc
	((driverEntryPtr *)(ret->overviewDrivers), 
	 sizeof(driverEntryPtr) * ret->num_overviewDrivers);
      driver = (driverEntryPtr) malloc(sizeof(driverEntry));
      if (driver == NULL) {
	fprintf(stderr,"Out of memory!\n");
	xmlFreeDoc(doc);
	exit(1);
      }
      ret->overviewDrivers[ret->num_overviewDrivers-1] = driver;
      memset(driver, 0, sizeof(driverEntry));
      if (debug) fprintf(stderr, "--> Parsing driver data\n");
      parseDriverEntry(doc, cur, driver, language, debug);
    } else if (!xmlStrcmp(cur->name, (const xmlChar *) "printer")) {
      if (debug) fprintf(stderr, "--> Parsing printer data\n");
      parseOverviewPrinter(doc, cur, ret, language, debug);
    } 
    cur = cur->next;
  }
  
  /* We succeeded, return the result */

  return(ret);
}

static comboDataPtr   /* O - C data structure of printer/driver combo */
parseComboFile(char *filename, /* I - Input file name, NULL: stdin */
	       xmlChar *language, /* I - User language */
	       int debug) { /* I - Debug mode flag */
  xmlDocPtr      doc;  /* Output of XML parser */
  comboDataPtr   ret;  /* C data structure of printer/driver combo */
  xmlNodePtr     cur;  /* XML node currently worked on */
  
  /*
   * build an XML tree from a file or stdin;
   */
  
  if (filename == NULL) {
    doc = parseXMLFromStdin();
  } else {
    doc = xmlParseFile(filename);
  }
  if (doc == NULL) return(NULL);
  
  /*
   * Check the document is of the right kind
   */
  
  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    fprintf(stderr,"Empty input document!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  
  if (xmlStrcmp(cur->name, (const xmlChar *) "foomatic")) {
    fprintf(stderr,"Input document is not a Foomatic combo XML file (no \"<foomatic>\" tag)!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  
  /*
   * Allocate the structure to be returned.
   */
  ret = (comboDataPtr) malloc(sizeof(comboData));
  if (ret == NULL) {
    fprintf(stderr,"Out of memory!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  memset(ret, 0, sizeof(comboData));
  ret->make = NULL;
  ret->driver = NULL;
  ret->num_args = 0;
  ret->args = NULL;

  /*
   * Now, walk through the tree.
   */

  /* On the first level we have the printer, the driver, 
     and the options */

  cur = cur->xmlChildrenNode;
  while (cur) {
    if (xmlIsBlankNode(cur)) {
      cur = cur->next;
      continue;
    }
    if (!xmlStrcmp(cur->name, (const xmlChar *) "printer")) {
      if (debug) fprintf(stderr, "--> Parsing printer data\n");
      parseComboPrinter(doc, cur, ret, language, debug);
    } else if (!xmlStrcmp(cur->name, (const xmlChar *) "driver")) {
      if (debug) fprintf(stderr, "--> Parsing driver data\n");
      parseComboDriver(doc, cur, ret, language, debug);
    } else if (!xmlStrcmp(cur->name, (const xmlChar *) "options")) {
      if (debug) fprintf(stderr, "--> Parsing option data\n");
      parseOptions(doc, cur, ret, language, debug);
    }
    cur = cur->next;
  }
  
  /* All sections present in combo XML? */

  if (ret->make == NULL) {
    fprintf(stderr,"\"<printer>\" tag not found!\n");
    exit(1);
  }

  if (ret->driver == NULL) {
    fprintf(stderr,"\"<driver>\" tag not found!\n");
    exit(1);
  }

  /* We succeeded, return the result */

  return(ret);
}

static printerEntryPtr     /* O - C data structure of printer entry */
parsePrinterFile(char *filename, /* I - Input file name, NULL: stdin */
		 xmlChar *language, /* I - User language */
		 int debug) { /* I - Debug mode flag */
  xmlDocPtr       doc;  /* Output of XML parser */
  printerEntryPtr ret;  /* C data structure of printer entry */
  xmlNodePtr      cur;  /* XML node currently worked on */
  
  /*
   * build an XML tree from a file or stdin;
   */
  
  if (filename == NULL) {
    doc = parseXMLFromStdin();
  } else {
    doc = xmlParseFile(filename);
  }
  if (doc == NULL) return(NULL);
  
  /*
   * Check the document is of the right kind
   */
  
  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    fprintf(stderr,"Empty input document!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  
  if (xmlStrcmp(cur->name, (const xmlChar *) "printer")) {
    fprintf(stderr,"Input document is not a Foomatic printer XML file (no \"<printer>\" tag)!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  
  /*
   * Allocate the structure to be returned.
   */
  ret = (printerEntryPtr) malloc(sizeof(printerEntry));
  if (ret == NULL) {
    fprintf(stderr,"Out of memory!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  memset(ret, 0, sizeof(printerEntry));

  /*
   * Now, walk through the tree.
   */

  if (debug) fprintf(stderr, "--> Parsing printer data\n");
  parsePrinterEntry(doc, cur, ret, language, debug);

  /* We succeeded, return the result */

  return(ret);
}

static driverEntryPtr     /* O - C data structure of driver entry */
parseDriverFile(char *filename, /* I - Input file name, NULL: stdin */
		xmlChar *language, /* I - User language */
		int debug) { /* I - Debug mode flag */
  xmlDocPtr       doc;  /* Output of XML parser */
  driverEntryPtr  ret;  /* C data structure of driver entry */
  xmlNodePtr      cur;  /* XML node currently worked on */
  
  /*
   * build an XML tree from a file or stdin;
   */
  
  if (filename == NULL) {
    doc = parseXMLFromStdin();
  } else {
    doc = xmlParseFile(filename);
  }
  if (doc == NULL) return(NULL);
  
  /*
   * Check the document is of the right kind
   */
  
  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    fprintf(stderr,"Empty input document!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  
  if (xmlStrcmp(cur->name, (const xmlChar *) "driver")) {
    fprintf(stderr,"Input document is not a Foomatic driver XML file (no \"<driver>\" tag)!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  
  /*
   * Allocate the structure to be returned.
   */
  ret = (driverEntryPtr) malloc(sizeof(driverEntry));
  if (ret == NULL) {
    fprintf(stderr,"Out of memory!\n");
    xmlFreeDoc(doc);
    return(NULL);
  }
  memset(ret, 0, sizeof(driverEntry));

  /*
   * Now, walk through the tree.
   */

  if (debug) fprintf(stderr, "--> Parsing driver data\n");
  parseDriverEntry(doc, cur, ret, language, debug);

  /* We succeeded, return the result */

  return(ret);
}

void
prepareComboData(comboDataPtr combo, /* I/O - Foomatic combo data parsed
					from XML input */
		 xmlChar **defaultsettings, /* User-supplied default option
					    settings */
		 int num_defaultsettings, /* number of default option
					     settings */
		 int debug) { /* Debug flag */

  int i, j; /* loop variables */
  xmlChar *option, *value, *s;/* User option and value, temporary pointer */

  /* Insert the default values (not choice IDs) for the enumerated 
     options */

  for (i = 0; i < combo->num_args; i ++) {
    for (j = 0; j < combo->args[i]->num_choices; j ++) {
      if (!xmlStrcmp(combo->args[i]->choices[j]->idx,
		     combo->args[i]->default_value)) {
	combo->args[i]->default_value = combo->args[i]->choices[j]->value;
	if (debug) 
	  fprintf(stderr,
		  "  Setting default value for enumerated option %s: %s\n",
		  combo->args[i]->name,
		  combo->args[i]->default_value);
      }
    }
  }
    
  /* Insert the user-supplied defaults */

  for (i = 0; i < num_defaultsettings; i ++) {
    s = strchr((char *)(defaultsettings[i]), '=');
    *s = (xmlChar)'\0';
    s ++;
    option = defaultsettings[i];
    value = s;
    for (j = 0; j < combo->num_args; j ++) {
      if (!xmlStrcmp(combo->args[j]->name, option)) {
	combo->args[j]->default_value = value;
	if (debug) fprintf(stderr,
			   "  Setting user default for option %s: %s\n",
			   combo->args[j]->name,
			   combo->args[j]->default_value);
      }
    }
  }

}

void
generateOverviewPerlData(overviewPtr overview, /* I/O - Foomatic overview 
						  data parsed from XML 
						  input */
			 int debug) { /* Debug flag */

  int i, j, k, l; /* loop variables */
  overviewPrinterPtr printer;
  
  printf("$VAR1 = [\n");
  for (i = 0; i < overview->num_overviewPrinters; i ++) {
    printer = overview->overviewPrinters[i];
    printf("          {\n");
    printf("            'id' => '%s',\n", printer->id);
    printf("            'make' => '%s',\n", printer->make);
    printf("            'model' => '%s',\n", printer->model);
    if (printer->general_ieee) {
      printf("            'general_ieee' => '%s',\n",
	     printer->general_ieee);
    }
    if (printer->general_mfg) {
      printf("            'general_mfg' => '%s',\n", printer->general_mfg);
    }
    if (printer->general_mdl) {
      printf("            'general_mdl' => '%s',\n", printer->general_mdl);
    }
    if (printer->general_des) {
      printf("            'general_des' => '%s',\n", printer->general_des);
    }
    if (printer->general_cmd) {
      printf("            'general_cmd' => '%s',\n", printer->general_cmd);
    }
    if (printer->par_ieee) {
      printf("            'par_ieee' => '%s',\n", printer->par_ieee);
    }
    if (printer->par_mfg) {
      printf("            'par_mfg' => '%s',\n", printer->par_mfg);
    }
    if (printer->par_mdl) {
      printf("            'par_mdl' => '%s',\n", printer->par_mdl);
    }
    if (printer->par_des) {
      printf("            'par_des' => '%s',\n", printer->par_des);
    }
    if (printer->par_cmd) {
      printf("            'par_cmd' => '%s',\n", printer->par_cmd);
    }
    if (printer->usb_ieee) {
      printf("            'usb_ieee' => '%s',\n", printer->usb_ieee);
    }
    if (printer->usb_mfg) {
      printf("            'usb_mfg' => '%s',\n", printer->usb_mfg);
    }
    if (printer->usb_mdl) {
      printf("            'usb_mdl' => '%s',\n", printer->usb_mdl);
    }
    if (printer->usb_des) {
      printf("            'usb_des' => '%s',\n", printer->usb_des);
    }
    if (printer->usb_cmd) {
      printf("            'usb_cmd' => '%s',\n", printer->usb_cmd);
    }
    if (printer->snmp_ieee) {
      printf("            'snmp_ieee' => '%s',\n", printer->snmp_ieee);
    }
    if (printer->snmp_mfg) {
      printf("            'snmp_mfg' => '%s',\n", printer->snmp_mfg);
    }
    if (printer->snmp_mdl) {
      printf("            'snmp_mdl' => '%s',\n", printer->snmp_mdl);
    }
    if (printer->snmp_des) {
      printf("            'snmp_des' => '%s',\n", printer->snmp_des);
    }
    if (printer->snmp_cmd) {
      printf("            'snmp_cmd' => '%s',\n", printer->snmp_cmd);
    }
    printf("            'functionality' => '%s',\n", 
	   printer->functionality);
    if (printer->unverified) {
      printf("            'unverified' => 1,\n");
    } else {
      printf("            'unverified' => 0,\n");
    }
    if (printer->noxmlentry) {
      printf("            'noxmlentry' => 1,\n");
    } else {
      printf("            'noxmlentry' => 0,\n");
    }
    if (printer->driver) {
      printf("            'driver' => '%s',\n", printer->driver);
    }
    if (printer->num_drivers > 0) {
      printf("            'drivers' => [\n");
      for (j = 0; j < printer->num_drivers; j ++)
	if (printer->drivers[j]->name != NULL)
	  printf("                           '%s',\n",
		 printer->drivers[j]->name);
      printf("                         ],\n");
      printf("            'driverproperties' => {\n");
      for (j = 0; j < printer->num_drivers; j ++) {
	for (k = 0; k < overview->num_overviewDrivers; k ++) {
	  if (!xmlStrcmp(overview->overviewDrivers[k]->name,
			 printer->drivers[j]->name)) break;
	}
	if ((k < overview->num_overviewDrivers) &&
	    (!xmlStrcmp(overview->overviewDrivers[k]->name,
			printer->drivers[j]->name))) {
	  printf("              '%s' => {\n",
		 printer->drivers[j]->name);
	  if (overview->overviewDrivers[k]->group != NULL) {
	    printf("                'group' => '%s',\n",
		   overview->overviewDrivers[k]->group);
	  }
	  if (overview->overviewDrivers[k]->url != NULL) {
	    printf("                'url' => '%s',\n",
		   overview->overviewDrivers[k]->url);
	  }
	  if (overview->overviewDrivers[k]->driver_obsolete != NULL) {
	    printf("                'obsolete' => '%s',\n",
		   overview->overviewDrivers[k]->driver_obsolete);
	  }
	  if (overview->overviewDrivers[k]->supplier != NULL) {
	    printf("                'supplier' => '%s',\n",
		   overview->overviewDrivers[k]->supplier);
	  }
	  if (overview->overviewDrivers[k]->manufacturersupplied != NULL) {
	    printf("                'manufacturersupplied' => '%s',\n",
		   overview->overviewDrivers[k]->manufacturersupplied);
	  }
	  if (overview->overviewDrivers[k]->license != NULL) {
	    printf("                'license' => '%s',\n",
		   overview->overviewDrivers[k]->license);
	  }
	  if (overview->overviewDrivers[k]->licensetext != NULL) {
	    printf("                'licensetext' => '%s',\n",
		   overview->overviewDrivers[k]->licensetext);
	  }
	  if (overview->overviewDrivers[k]->origlicensetext != NULL) {
	    printf("                'origlicensetext' => '%s',\n",
		   overview->overviewDrivers[k]->origlicensetext);
	  }
	  if (overview->overviewDrivers[k]->licenselink != NULL) {
	    printf("                'licenselink' => '%s',\n",
		   overview->overviewDrivers[k]->licenselink);
	  }
	  if (overview->overviewDrivers[k]->origlicenselink != NULL) {
	    printf("                'origlicenselink' => '%s',\n",
		   overview->overviewDrivers[k]->origlicenselink);
	  }
	  if (overview->overviewDrivers[k]->free != NULL) {
	    printf("                'free' => '%s',\n",
		   overview->overviewDrivers[k]->free);
	  }
	  if (overview->overviewDrivers[k]->patents != NULL) {
	    printf("                'patents' => '%s',\n",
		   overview->overviewDrivers[k]->patents);
	  }
	  if (overview->overviewDrivers[k]->num_supportcontacts != 0) {
	    printf("                'supportcontacts' => [\n");
	    for (l = 0;
		 l < overview->overviewDrivers[k]->num_supportcontacts; l ++) {
	      if (overview->overviewDrivers[k]->supportcontacturls[l] != 
		  NULL) {
		printf("                  {\n");
		printf("                    'description' => '%s',\n",
		       overview->overviewDrivers[k]->supportcontacts[l]);
		if (overview->overviewDrivers[k]->supportcontacturls[l]
		    != NULL)
		  printf("                    'url' => '%s',\n",
			 overview->overviewDrivers[k]->supportcontacturls[l]);
		printf("                    'level' => '%s',\n",
		       overview->overviewDrivers[k]->supportcontactlevels[l]);
		printf("                  },\n");
	      }
	    }
	    printf("                ],\n");
	  }
	  if (overview->overviewDrivers[k]->shortdescription != NULL) {
	    printf("                'shortdescription' => '%s',\n",
		   overview->overviewDrivers[k]->shortdescription);
	  }
	  if (overview->overviewDrivers[k]->locales != NULL) {
	    printf("                'locales' => '%s',\n",
		   overview->overviewDrivers[k]->locales);
	  }
	  if (overview->overviewDrivers[k]->num_packages != 0) {
	    printf("                'packages' => [\n");
	    for (l = 0;
		 l < overview->overviewDrivers[k]->num_packages; l ++) {
	      if (overview->overviewDrivers[k]->packageurls[l] != 
		  NULL) {
		printf("                  {\n");
		printf("                    'url' => '%s',\n",
		       overview->overviewDrivers[k]->packageurls[l]);
		if (overview->overviewDrivers[k]->packagescopes[l]
		    != NULL)
		  printf("                    'scope' => '%s',\n",
			 overview->overviewDrivers[k]->packagescopes[l]);
		if (overview->overviewDrivers[k]->packagefingerprints[l]
		    != NULL)
		  printf("                    'fingerprint' => '%s',\n",
			 overview->overviewDrivers[k]->packagefingerprints[l]);
		printf("                  },\n");
	      }
	    }
	    printf("                ],\n");
	  }
	  if (overview->overviewDrivers[k]->num_requires != 0) {
	    printf("                'requires' => [\n");
	    for (l = 0;
		 l < overview->overviewDrivers[k]->num_requires; l ++) {
	      if (overview->overviewDrivers[k]->requires[l] != 
		  NULL) {
		printf("                  {\n");
		printf("                    'driver' => '%s',\n",
		       overview->overviewDrivers[k]->requires[l]);
		if (overview->overviewDrivers[k]->requiresversion[l]
		    != NULL)
		  printf("                    'version' => '%s',\n",
			 overview->overviewDrivers[k]->requiresversion[l]);
		printf("                  },\n");
	      }
	    }
	    printf("                ],\n");
	  }
	  if (overview->overviewDrivers[k]->driver_type != NULL) {
	    printf("                'type' => '%s',\n",
		   overview->overviewDrivers[k]->driver_type);
	  }
	  if (printer->drivers[j]->excmaxresx != NULL) {
	    printf("                'drvmaxresx' => '%s',\n",
		   printer->drivers[j]->excmaxresx);
	  } else if (overview->overviewDrivers[k]->maxresx != NULL) {
	    printf("                'drvmaxresx' => '%s',\n",
		   overview->overviewDrivers[k]->maxresx);
	  }
	  if (printer->drivers[j]->excmaxresy != NULL) {
	    printf("                'drvmaxresy' => '%s',\n",
		   printer->drivers[j]->excmaxresy);
	  } else if (overview->overviewDrivers[k]->maxresy != NULL) {
	    printf("                'drvmaxresy' => '%s',\n",
		   overview->overviewDrivers[k]->maxresy);
	  }
	  if (printer->drivers[j]->exccolor != NULL) {
	    printf("                'drvcolor' => '%s',\n",
		   printer->drivers[j]->exccolor);
	  } else if (overview->overviewDrivers[k]->color != NULL) {
	    printf("                'drvcolor' => '%s',\n",
		   overview->overviewDrivers[k]->color);
	  }
	  if (printer->drivers[j]->exctext != NULL) {
	    printf("                'text' => '%s',\n",
		   printer->drivers[j]->exctext);
	  } else if (overview->overviewDrivers[k]->text != NULL) {
	    printf("                'text' => '%s',\n",
		   overview->overviewDrivers[k]->text);
	  }
	  if (printer->drivers[j]->exclineart != NULL) {
	    printf("                'lineart' => '%s',\n",
		   printer->drivers[j]->exclineart);
	  } else if (overview->overviewDrivers[k]->lineart != NULL) {
	    printf("                'lineart' => '%s',\n",
		   overview->overviewDrivers[k]->lineart);
	  }
	  if (printer->drivers[j]->excgraphics != NULL) {
	    printf("                'graphics' => '%s',\n",
		   printer->drivers[j]->excgraphics);
	  } else if (overview->overviewDrivers[k]->graphics != NULL) {
	    printf("                'graphics' => '%s',\n",
		   overview->overviewDrivers[k]->graphics);
	  }
	  if (printer->drivers[j]->excphoto != NULL) {
	    printf("                'photo' => '%s',\n",
		   printer->drivers[j]->excphoto);
	  } else if (overview->overviewDrivers[k]->photo != NULL) {
	    printf("                'photo' => '%s',\n",
		   overview->overviewDrivers[k]->photo);
	  }
	  if (printer->drivers[j]->excload != NULL) {
	    printf("                'load' => '%s',\n",
		   printer->drivers[j]->excload);
	  } else if (overview->overviewDrivers[k]->load != NULL) {
	    printf("                'load' => '%s',\n",
		   overview->overviewDrivers[k]->load);
	  }
	  if (printer->drivers[j]->excspeed != NULL) {
	    printf("                'speed' => '%s',\n",
		   printer->drivers[j]->excspeed);
	  } else if (overview->overviewDrivers[k]->speed != NULL) {
	    printf("                'speed' => '%s',\n",
		   overview->overviewDrivers[k]->speed);
	  }
	  printf("              },\n");
	}
      }
      printf("            },\n");
    } else {
      printf("            'drivers' => [],\n");
    }
    if (printer->num_ppdfiles > 0) {
      printf("            'ppds' => [\n");
      for (j = 0; j < printer->num_ppdfiles; j ++)
	if ((printer->ppdfiles[j]->driver != NULL) &&
	    (printer->ppdfiles[j]->filename != NULL)) {
	  printf("                        {\n");
	  printf("                          'driver' => '%s',\n",
		 printer->ppdfiles[j]->driver);
	  printf("                          'ppdfile' => '%s',\n",
		 printer->ppdfiles[j]->filename);
	  printf("                        },\n");
	}
      printf("                      ],\n");
    }
    printf("          },\n");
  }
  printf("        ];\n");

}

void
generateMarginsPerlData(marginsPtr margins, /* I/O - Foomatic margins data
					       parsed from XML input */
			int debug) { /* Debug flag */

  int i; /* loop variable */
  
  for (i = 0; i < margins->num_marginRecords; i ++) {
    if (margins->marginRecords[i]->pagesize) {
      printf("    '%s' => {\n", margins->marginRecords[i]->pagesize);
    } else {
      printf("    '_general' => {\n");
    }
    if (margins->marginRecords[i]->unit) {
      printf("      'unit' => '%s',\n", margins->marginRecords[i]->unit);
    }
    if (margins->marginRecords[i]->absolute) {
      printf("      'absolute' => '%s',\n", 
	     margins->marginRecords[i]->absolute);
    }
    if (margins->marginRecords[i]->left) {
      printf("      'left' => '%s',\n", margins->marginRecords[i]->left);
    }
    if (margins->marginRecords[i]->right) {
      printf("      'right' => '%s',\n", margins->marginRecords[i]->right);
    }
    if (margins->marginRecords[i]->top) {
      printf("      'top' => '%s',\n", margins->marginRecords[i]->top);
    }
    if (margins->marginRecords[i]->bottom) {
      printf("      'bottom' => '%s',\n", margins->marginRecords[i]->bottom);
    }
    printf("    },\n");
  }
}

void
generateComboPerlData(comboDataPtr combo, /* I/O - Foomatic combo data
					     parsed from XML input */
		      int debug) { /* Debug flag */

  int i, j; /* loop variables */
  int haspsdriver = 0; /* Is the "Postscript" driver in the printer's
			  driver list? */
  
  printf("$VAR1 = {\n");
  printf("  'id' => '%s',\n", combo->id);
  printf("  'make' => '%s',\n", combo->make);
  printf("  'model' => '%s',\n", combo->model);
  if (combo->recdriver) {
    printf("  'recdriver' => '%s',\n", combo->recdriver);
  } else {
    printf("  'recdriver' => undef,\n");
  }
  if ((combo->num_drivers > 0) || (combo->ppdurl)) {
    printf("  'drivers' => [\n");
    for (i = 0; i < combo->num_drivers; i ++) {
      printf("                 {\n");
      if (combo->drivers[i]->name) {
	if (strncmp(combo->drivers[i]->name, "Postscript", 10))
	  haspsdriver = 1;
	printf("                   'name' => '%s',\n",
	       combo->drivers[i]->name);
	printf("                   'id' => '%s',\n",
	       combo->drivers[i]->name);
      }
      if (combo->drivers[i]->ppd) {
	printf("                   'ppd' => '%s',\n",
	       combo->drivers[i]->ppd);
      }
      if (combo->drivers[i]->comment) {
	printf("                   'comment' => '%s',\n",
	       combo->drivers[i]->comment);
      }
      printf("                 },\n");
    }
    if ((combo->ppdurl) && !haspsdriver) {
      printf("                 {\n");
      printf("                   'name' => '%s',\n",
	     "Postscript");
      printf("                   'id' => '%s',\n",
	     "Postscript");
      printf("                   'ppd' => '%s',\n",
	     combo->ppdurl);
      printf("                 },\n");
    }
    printf("               ],\n");
  }
  if (combo->pcmodel) {
    printf("  'pcmodel' => '%s',\n", combo->pcmodel);
  } else {
    printf("  'pcmodel' => undef,\n");
  }
  if (combo->ppdurl) {
    printf("  'ppdurl' => '%s',\n", combo->ppdurl);
  }
  printf("  'color' => %s,\n", combo->color);
  printf("  'ascii' => %s,\n", combo->ascii);
  printf("  'pjl' => %s,\n", combo->pjl);
  if (combo->printerppdentry) {
    printf("  'printerppdentry' => '%s',\n", combo->printerppdentry);
  } else {
    printf("  'printerppdentry' => undef,\n");
  }
  if (combo->printermargins) {
    printf("  'printermargins' => {\n");
    generateMarginsPerlData(combo->printermargins, debug);
    printf("  },\n");
  }
  if (combo->general_ieee) {
    printf("  'pnp_ieee' => '%s',\n", combo->general_ieee);
    printf("  'general_ieee' => '%s',\n", combo->general_ieee);
  } else {
    printf("  'pnp_ieee' => undef,\n");
    printf("  'general_ieee' => undef,\n");
  }
  if (combo->general_mfg) {
    printf("  'pnp_mfg' => '%s',\n", combo->general_mfg);
    printf("  'general_mfg' => '%s',\n", combo->general_mfg);
  } else {
    printf("  'pnp_mfg' => undef,\n");
    printf("  'general_mfg' => undef,\n");
  }
  if (combo->general_mdl) {
    printf("  'pnp_mdl' => '%s',\n", combo->general_mdl);
    printf("  'general_mdl' => '%s',\n", combo->general_mdl);
  } else {
    printf("  'pnp_mdl' => undef,\n");
    printf("  'general_mdl' => undef,\n");
  }
  if (combo->general_des) {
    printf("  'pnp_des' => '%s',\n", combo->general_des);
    printf("  'general_des' => '%s',\n", combo->general_des);
  } else {
    printf("  'pnp_des' => undef,\n");
    printf("  'general_des' => undef,\n");
  }
  if (combo->general_cmd) {
    printf("  'pnp_cmd' => '%s',\n", combo->general_cmd);
    printf("  'general_cmd' => '%s',\n", combo->general_cmd);
  } else {
    printf("  'pnp_cmd' => undef,\n");
    printf("  'general_cmd' => undef,\n");
  }
  if (combo->par_ieee) {
    printf("  'par_ieee' => '%s',\n", combo->par_ieee);
  } else {
    printf("  'par_ieee' => undef,\n");
  }
  if (combo->par_mfg) {
    printf("  'par_mfg' => '%s',\n", combo->par_mfg);
  } else {
    printf("  'par_mfg' => undef,\n");
  }
  if (combo->par_mdl) {
    printf("  'par_mdl' => '%s',\n", combo->par_mdl);
  } else {
    printf("  'par_mdl' => undef,\n");
  }
  if (combo->par_des) {
    printf("  'par_des' => '%s',\n", combo->par_des);
  } else {
    printf("  'par_des' => undef,\n");
  }
  if (combo->par_cmd) {
    printf("  'par_cmd' => '%s',\n", combo->par_cmd);
  } else {
    printf("  'par_cmd' => undef,\n");
  }
  if (combo->usb_ieee) {
    printf("  'usb_ieee' => '%s',\n", combo->usb_ieee);
  } else {
    printf("  'usb_ieee' => undef,\n");
  }
  if (combo->usb_mfg) {
    printf("  'usb_mfg' => '%s',\n", combo->usb_mfg);
  } else {
    printf("  'usb_mfg' => undef,\n");
  }
  if (combo->usb_mdl) {
    printf("  'usb_mdl' => '%s',\n", combo->usb_mdl);
  } else {
    printf("  'usb_mdl' => undef,\n");
  }
  if (combo->usb_des) {
    printf("  'usb_des' => '%s',\n", combo->usb_des);
  } else {
    printf("  'usb_des' => undef,\n");
  }
  if (combo->usb_cmd) {
    printf("  'usb_cmd' => '%s',\n", combo->usb_cmd);
  } else {
    printf("  'usb_cmd' => undef,\n");
  }
  if (combo->snmp_ieee) {
    printf("  'snmp_ieee' => '%s',\n", combo->snmp_ieee);
  } else {
    printf("  'snmp_ieee' => undef,\n");
  }
  if (combo->snmp_mfg) {
    printf("  'snmp_mfg' => '%s',\n", combo->snmp_mfg);
  } else {
    printf("  'snmp_mfg' => undef,\n");
  }
  if (combo->snmp_mdl) {
    printf("  'snmp_mdl' => '%s',\n", combo->snmp_mdl);
  } else {
    printf("  'snmp_mdl' => undef,\n");
  }
  if (combo->snmp_des) {
    printf("  'snmp_des' => '%s',\n", combo->snmp_des);
  } else {
    printf("  'snmp_des' => undef,\n");
  }
  if (combo->snmp_cmd) {
    printf("  'snmp_cmd' => '%s',\n", combo->snmp_cmd);
  } else {
    printf("  'snmp_cmd' => undef,\n");
  }
  printf("  'driver' => '%s',\n", combo->driver);
  if (combo->driver_group) {
    printf("  'group' => '%s',\n", combo->driver_group);
  }
  if (combo->pcdriver) {
    printf("  'pcdriver' => '%s',\n", combo->pcdriver);
  } else {
    printf("  'pcdriver' => undef,\n");
  }
  printf("  'type' => '%s',\n", combo->driver_type);
  if (combo->driver_comment) {
    printf("  'comment' => '%s',\n", combo->driver_comment);
  } else {
    printf("  'comment' => undef,\n");
  }
  if (combo->url) {
    printf("  'url' => '%s',\n", combo->url);
  } else {
    printf("  'url' => undef,\n");
  }
  if (combo->driver_obsolete) {
    printf("  'obsolete' => '%s',\n", combo->driver_obsolete);
  }
  if (combo->supplier != NULL) {
    printf("  'supplier' => '%s',\n",
	   combo->supplier);
  }
  if (combo->manufacturersupplied != NULL) {
    printf("  'manufacturersupplied' => '%s',\n",
	   combo->manufacturersupplied);
  }
  if (combo->license != NULL) {
    printf("  'license' => '%s',\n",
	   combo->license);
  }
  if (combo->licensetext != NULL) {
    printf("  'licensetext' => '%s',\n",
	   combo->licensetext);
  }
  if (combo->origlicensetext != NULL) {
    printf("  'origlicensetext' => '%s',\n",
	   combo->origlicensetext);
  }
  if (combo->licenselink != NULL) {
    printf("  'licenselink' => '%s',\n",
	   combo->licenselink);
  }
  if (combo->origlicenselink != NULL) {
    printf("  'origlicenselink' => '%s',\n",
	   combo->origlicenselink);
  }
  if (combo->free != NULL) {
    printf("  'free' => '%s',\n",
	   combo->free);
  }
  if (combo->patents != NULL) {
    printf("  'patents' => '%s',\n",
	   combo->patents);
  }
  if (combo->num_supportcontacts != 0) {
    printf("  'supportcontacts' => [\n");
    for (i = 0;
	 i < combo->num_supportcontacts; i ++) {
      if (combo->supportcontacturls[i] != 
	  NULL) {
	printf("    {\n");
	printf("      'description' => '%s',\n",
	       combo->supportcontacts[i]);
	if (combo->supportcontacturls[i]
	    != NULL)
	  printf("      'url' => '%s',\n",
		 combo->supportcontacturls[i]);
	printf("      'level' => '%s',\n",
	       combo->supportcontactlevels[i]);
	printf("    },\n");
      }
    }
    printf("  ],\n");
  }
  if (combo->shortdescription != NULL) {
    printf("  'shortdescription' => '%s',\n",
	   combo->shortdescription);
  }
  if (combo->locales != NULL) {
    printf("  'locales' => '%s',\n",
	   combo->locales);
  }
  if (combo->num_packages != 0) {
    printf("  'packages' => [\n");
    for (i = 0;
	 i < combo->num_packages; i ++) {
      if (combo->packageurls[i] != 
	  NULL) {
	printf("    {\n");
	printf("      'url' => '%s',\n",
	       combo->packageurls[i]);
	if (combo->packagescopes[i]
	    != NULL)
	  printf("      'scope' => '%s',\n",
		 combo->packagescopes[i]);
	if (combo->packagefingerprints[i]
	    != NULL)
	  printf("      'fingerprint' => '%s',\n",
		 combo->packagefingerprints[i]);
	printf("    },\n");
      }
    }
    printf("  ],\n");
  }
  if (combo->excmaxresx != NULL) {
    printf("  'drvmaxresx' => '%s',\n",
	   combo->excmaxresx);
  } else if (combo->drvmaxresx != NULL) {
    printf("  'drvmaxresx' => '%s',\n",
	   combo->drvmaxresx);
  }
  if (combo->excmaxresy != NULL) {
    printf("  'drvmaxresy' => '%s',\n",
	   combo->excmaxresy);
  } else if (combo->drvmaxresy != NULL) {
    printf("  'drvmaxresy' => '%s',\n",
	   combo->drvmaxresy);
  }
  if (combo->exccolor != NULL) {
    printf("  'drvcolor' => '%s',\n",
	   combo->exccolor);
  } else if (combo->drvcolor != NULL) {
    printf("  'drvcolor' => '%s',\n",
	   combo->drvcolor);
  }
  if (combo->exctext != NULL) {
    printf("  'text' => '%s',\n",
	   combo->exctext);
  } else if (combo->text != NULL) {
    printf("  'text' => '%s',\n",
	   combo->text);
  }
  if (combo->exclineart != NULL) {
    printf("  'lineart' => '%s',\n",
	   combo->exclineart);
  } else if (combo->lineart != NULL) {
    printf("  'lineart' => '%s',\n",
	   combo->lineart);
  }
  if (combo->excgraphics != NULL) {
    printf("  'graphics' => '%s',\n",
	   combo->excgraphics);
  } else if (combo->graphics != NULL) {
    printf("  'graphics' => '%s',\n",
	   combo->graphics);
  }
  if (combo->excphoto != NULL) {
    printf("  'photo' => '%s',\n",
	   combo->excphoto);
  } else if (combo->photo != NULL) {
    printf("  'photo' => '%s',\n",
	   combo->photo);
  }
  if (combo->excload != NULL) {
    printf("  'load' => '%s',\n",
	   combo->excload);
  } else if (combo->load != NULL) {
    printf("  'load' => '%s',\n",
	   combo->load);
  }
  if (combo->excspeed != NULL) {
    printf("  'speed' => '%s',\n",
	   combo->excspeed);
  } else if (combo->speed != NULL) {
    printf("  'speed' => '%s',\n",
	   combo->speed);
  }
  if (combo->num_requires != 0) {
    printf("  'requires' => [\n");
    for (i = 0;
	 i < combo->num_requires; i ++) {
      if (combo->requires[i] != 
	  NULL) {
	printf("    {\n");
	printf("      'driver' => '%s',\n",
	       combo->requires[i]);
	if (combo->requiresversion[i]
	    != NULL)
	  printf("      'version' => '%s',\n",
		 combo->requiresversion[i]);
	printf("    },\n");
      }
    }
    printf("  ],\n");
  }
  if (combo->cmd) {
    printf("  'cmd' => '%s',\n", combo->cmd);
  } else {
    printf("  'cmd' => undef,\n");
  }
  if (combo->cmd_pdf) {
    printf("  'cmd_pdf' => '%s',\n", combo->cmd_pdf);
  } else {
    printf("  'cmd_pdf' => undef,\n");
  }
  if (combo->nopjl) {
    printf("  'drivernopjl' => %s,\n", combo->nopjl);
  } else {
    printf("  'drivernopjl' => 0,\n");
  }
  if (combo->nopageaccounting) {
    printf("  'drivernopageaccounting' => %s,\n", combo->nopageaccounting);
  } else {
    printf("  'drivernopageaccounting' => 0,\n");
  }
  if (combo->driverppdentry) {
    printf("  'driverppdentry' => '%s',\n", combo->driverppdentry);
  } else {
    printf("  'driverppdentry' => undef,\n");
  }
  if (combo->comboppdentry) {
    printf("  'comboppdentry' => '%s',\n", combo->comboppdentry);
  } else {
    printf("  'comboppdentry' => undef,\n");
  }
  if (combo->drivermargins) {
    printf("  'drivermargins' => {\n");
    generateMarginsPerlData(combo->drivermargins, debug);
    printf("  },\n");
  }
  if (combo->combomargins) {
    printf("  'combomargins' => {\n");
    generateMarginsPerlData(combo->combomargins, debug);
    printf("  },\n");
  }
  if (combo->maxspot > 0) {
    printf("  'maxspot' => '%s',\n", combo->maxspot);
  } else {
    printf("  'maxspot' => 'A',\n");
  }
  printf("  'args_byname' => {\n");
  for (i = 0; i < combo->num_args; i ++) {
    printf("    '%s' => {},\n", combo->args[i]->name);
  }
  printf("  },\n");
  printf("  'args' => [\n");
  for (i = 0; i < combo->num_args; i ++) {
    printf("    {\n");
    printf("      'name' => '%s',\n", combo->args[i]->name);
    if (combo->args[i]->name_false) {
      printf("      'name_false' => '%s',\n", combo->args[i]->name_false);
    }
    printf("      'comment' => '%s',\n", combo->args[i]->comment);
    printf("      'idx' => '%s',\n", combo->args[i]->idx);
    printf("      'type' => '%s',\n", combo->args[i]->option_type);
    printf("      'style' => '%s',\n", combo->args[i]->style);
    if (combo->args[i]->substyle) {
      printf("      'substyle' => '%s',\n", combo->args[i]->substyle);
    }
    printf("      'spot' => '%s',\n", combo->args[i]->spot);
    printf("      'order' => '%s',\n", combo->args[i]->order);
    if (combo->args[i]->section) {
      printf("      'section' => '%s',\n", combo->args[i]->section);
    }
    if (combo->args[i]->grouppath) {
      printf("      'group' => '%s',\n", combo->args[i]->grouppath);
    }
    if (combo->args[i]->proto) {
      printf("      'proto' => '%s',\n", combo->args[i]->proto);
    }
    if (combo->args[i]->required) {
      printf("      'required' => 1,\n");
    }
    if (combo->args[i]->min_value) {
      printf("      'min' => '%s',\n", combo->args[i]->min_value);
    }
    if (combo->args[i]->max_value) {
      printf("      'max' => '%s',\n", combo->args[i]->max_value);
    }
    if (combo->args[i]->max_length) {
      printf("      'maxlength' => '%s',\n", combo->args[i]->max_length);
    }
    if (combo->args[i]->allowed_chars) {
      printf("      'allowedchars' => '%s',\n", combo->args[i]->allowed_chars);
    }
    if (combo->args[i]->allowed_regexp) {
      printf("      'allowedregexp' => '%s',\n",
	     combo->args[i]->allowed_regexp);
    }
    if (combo->args[i]->default_value) {
      printf("      'default' => '%s',\n", combo->args[i]->default_value);
    } else {
      printf("      'default' => 'None',\n");
    }
    if (combo->args[i]->num_choices > 0) {
      printf("      'vals_byname' => {\n");
      for (j = 0; j < combo->args[i]->num_choices; j ++) {
	if (combo->args[i]->choices[j]->value == NULL) {
	  combo->args[i]->choices[j]->value = "None";
	}
	printf("        '%s' => {\n", combo->args[i]->choices[j]->value);
	printf("          'value' => '%s',\n", 
	       combo->args[i]->choices[j]->value);
	if (combo->args[i]->choices[j]->comment) {
	  printf("          'comment' => '%s',\n",
		 combo->args[i]->choices[j]->comment);
	}
	printf("          'idx' => '%s',\n",
	       combo->args[i]->choices[j]->idx);
	if (combo->args[i]->choices[j]->driverval) {
	  printf("          'driverval' => '%s'\n",
		 combo->args[i]->choices[j]->driverval);
	} else {
	  printf("          'driverval' => ''\n");
	}
	printf("        },\n");
      }
      printf("      },\n");
      printf("      'vals' => [\n");
      for (j = 0; j < combo->args[i]->num_choices; j ++) {
	printf("        {},\n");
      }
      printf("      ]\n");
    }
    printf("    },\n");
  }
  printf("  ]\n");
  printf("};\n");
  for (i = 0; i < combo->num_args; i ++) {
    for (j = 0; j < combo->args[i]->num_choices; j ++) {
      printf("$VAR1->{'args'}[%d]{'vals'}[%d] = $VAR1->{'args'}[%d]{'vals_byname'}{'%s'};\n",
	     i, j, i, combo->args[i]->choices[j]->value);
    }
  }
  for (i = 0; i < combo->num_args; i ++) {
    printf("$VAR1->{'args_byname'}{'%s'} = $VAR1->{'args'}[%d];\n",
	   combo->args[i]->name, i);
  }

}

void
generatePrinterPerlData(printerEntryPtr printer, /* I/O - Foomatic printer 
						    data parsed from XML 
						    input */
			int debug) { /* Debug flag */

  int i; /* loop variable */
  int haspsdriver = 0; /* Is the "Postscript" driver in the printer's
			  driver list? */

  printf("$VAR1 = {\n");
  printf("  'id' => '%s',\n", printer->id);
  printf("  'make' => '%s',\n", printer->make);
  printf("  'model' => '%s',\n", printer->model);
  if (printer->printer_type) {
    printf("  'type' => '%s',\n", printer->printer_type);
  }
  if (printer->color) {
    printf("  'color' => '%s',\n", printer->color);
  }
  if (printer->maxxres) {
    printf("  'maxxres' => '%s',\n", printer->maxxres);
  }
  if (printer->maxyres) {
    printf("  'maxyres' => '%s',\n", printer->maxyres);
  }
  if (printer->printerppdentry) {
    printf("  'ppdentry' => '%s',\n", printer->printerppdentry);
  } else {
    printf("  'ppdentry' => undef,\n");
  }
  if (printer->printermargins) {
    printf("  'margins' => {\n");
    generateMarginsPerlData(printer->printermargins, debug);
    printf("  },\n");
  }
  if (printer->refill) {
    printf("  'refill' => '%s',\n", printer->refill);
  }
  if (printer->ascii) {
    printf("  'ascii' => '%s',\n", printer->ascii);
  }
  if (printer->pjl) {
    printf("  'pjl' => '%s',\n", printer->pjl);
  }
  if (printer->num_languages > 0) {
    printf("  'languages' => [\n");
    for (i = 0; i < printer->num_languages; i ++) {
      printf("                   {\n");
      printf("                     'name' => '%s',\n",
	     printer->languages[i]->name);
      printf("                     'level' => '%s',\n",
	     printer->languages[i]->level);
      printf("                   },\n");
    }
    printf("                 ],\n");
  }
  if (printer->ppdurl) {
    printf("  'ppdurl' => '%s',\n", printer->ppdurl);
  }
  if (printer->general_ieee) {
    printf("  'general_ieee' => '%s',\n", printer->general_ieee);
  }
  if (printer->general_mfg) {
    printf("  'general_mfg' => '%s',\n", printer->general_mfg);
  }
  if (printer->general_mdl) {
    printf("  'general_mdl' => '%s',\n", printer->general_mdl);
  }
  if (printer->general_des) {
    printf("  'general_des' => '%s',\n", printer->general_des);
  }
  if (printer->general_cmd) {
    printf("  'general_cmd' => '%s',\n", printer->general_cmd);
  }
  if (printer->par_ieee) {
    printf("  'par_ieee' => '%s',\n", printer->par_ieee);
  }
  if (printer->par_mfg) {
    printf("  'par_mfg' => '%s',\n", printer->par_mfg);
  }
  if (printer->par_mdl) {
    printf("  'par_mdl' => '%s',\n", printer->par_mdl);
  }
  if (printer->par_des) {
    printf("  'par_des' => '%s',\n", printer->par_des);
  }
  if (printer->par_cmd) {
    printf("  'par_cmd' => '%s',\n", printer->par_cmd);
  }
  if (printer->usb_ieee) {
    printf("  'usb_ieee' => '%s',\n", printer->usb_ieee);
  }
  if (printer->usb_mfg) {
    printf("  'usb_mfg' => '%s',\n", printer->usb_mfg);
  }
  if (printer->usb_mdl) {
    printf("  'usb_mdl' => '%s',\n", printer->usb_mdl);
  }
  if (printer->usb_des) {
    printf("  'usb_des' => '%s',\n", printer->usb_des);
  }
  if (printer->usb_cmd) {
    printf("  'usb_cmd' => '%s',\n", printer->usb_cmd);
  }
  if (printer->snmp_ieee) {
    printf("  'snmp_ieee' => '%s',\n", printer->snmp_ieee);
  }
  if (printer->snmp_mfg) {
    printf("  'snmp_mfg' => '%s',\n", printer->snmp_mfg);
  }
  if (printer->snmp_mdl) {
    printf("  'snmp_mdl' => '%s',\n", printer->snmp_mdl);
  }
  if (printer->snmp_des) {
    printf("  'snmp_des' => '%s',\n", printer->snmp_des);
  }
  if (printer->snmp_cmd) {
    printf("  'snmp_cmd' => '%s',\n", printer->snmp_cmd);
  }
  if (printer->functionality) {
    printf("  'functionality' => '%s',\n", printer->functionality);
  }
  if (printer->driver) {
    printf("  'driver' => '%s',\n", printer->driver);
  }
  if ((printer->num_drivers > 0) || (printer->ppdurl)) {
    printf("  'drivers' => [\n");
    for (i = 0; i < printer->num_drivers; i ++) {
      printf("                 {\n");
      if (printer->drivers[i]->name) {
	if (strncmp(printer->drivers[i]->name, "Postscript", 10))
	  haspsdriver = 1;
	printf("                   'name' => '%s',\n",
	       printer->drivers[i]->name);
	printf("                   'id' => '%s',\n",
	       printer->drivers[i]->name);
      }
      if (printer->drivers[i]->ppd) {
	printf("                   'ppd' => '%s',\n",
	       printer->drivers[i]->ppd);
      }
      if (printer->drivers[i]->comment) {
	printf("                   'comment' => '%s',\n",
	       printer->drivers[i]->comment);
      }
      printf("                 },\n");
    }
    if ((printer->ppdurl) && !haspsdriver) {
      printf("                 {\n");
      printf("                   'name' => '%s',\n",
	     "Postscript");
      printf("                   'id' => '%s',\n",
	     "Postscript");
      printf("                   'ppd' => '%s',\n",
	     printer->ppdurl);
      printf("                 },\n");
    }
    printf("               ],\n");
  }
  if (printer->unverified) {
    printf("  'unverified' => '%s',\n", printer->unverified);
  }
  if (printer->noxmlentry) {
    printf("  'noxmlentry' => '%s',\n", printer->noxmlentry);
  }
  if (printer->url) {
    printf("  'url' => '%s',\n", printer->url);
  }
  if (printer->contriburl) {
    printf("  'contriburl' => '%s',\n", printer->contriburl);
  }
  if (printer->comment) {
    printf("  'comment' => '%s',\n", printer->comment);
  }
  printf("};\n");

}

void
generateDriverPerlData(driverEntryPtr driver, /* I/O - Foomatic driver
						 data parsed from XML 
						 input */
		       int debug) { /* Debug flag */

  int i; /* loop variable */
  
  xmlChar *id;
  xmlChar *name;
  xmlChar *url;
  xmlChar *driver_type;
  xmlChar *cmd;
  xmlChar *comment;
  int     num_printers;
  xmlChar **printers;
  printf("$VAR1 = {\n");
  printf("  'name' => '%s',\n", driver->name);
  if (driver->group) {
    printf("  'group' => '%s',\n", driver->group);
  }
  if (driver->url) {
    printf("  'url' => '%s',\n", driver->url);
  }
  if (driver->driver_obsolete) {
    printf("  'obsolete' => '%s',\n", driver->driver_obsolete);
  }
  if (driver->supplier != NULL) {
    printf("  'supplier' => '%s',\n",
	   driver->supplier);
  }
  if (driver->manufacturersupplied != NULL) {
    printf("  'manufacturersupplied' => '%s',\n",
	   driver->manufacturersupplied);
  }
  if (driver->license != NULL) {
    printf("  'license' => '%s',\n",
	   driver->license);
  }
  if (driver->licensetext != NULL) {
    printf("  'licensetext' => '%s',\n",
	   driver->licensetext);
  }
  if (driver->origlicensetext != NULL) {
    printf("  'origlicensetext' => '%s',\n",
	   driver->origlicensetext);
  }
  if (driver->licenselink != NULL) {
    printf("  'licenselink' => '%s',\n",
	   driver->licenselink);
  }
  if (driver->origlicenselink != NULL) {
    printf("  'origlicenselink' => '%s',\n",
	   driver->origlicenselink);
  }
  if (driver->free != NULL) {
    printf("  'free' => '%s',\n",
	   driver->free);
  }
  if (driver->patents != NULL) {
    printf("  'patents' => '%s',\n",
	   driver->patents);
  }
  if (driver->num_supportcontacts != 0) {
    printf("  'supportcontacts' => [\n");
    for (i = 0;
	 i < driver->num_supportcontacts; i ++) {
      if (driver->supportcontacturls[i] != 
	  NULL) {
	printf("    {\n");
	printf("      'description' => '%s',\n",
	       driver->supportcontacts[i]);
	if (driver->supportcontacturls[i]
	    != NULL)
	  printf("      'url' => '%s',\n",
		 driver->supportcontacturls[i]);
	printf("      'level' => '%s',\n",
	       driver->supportcontactlevels[i]);
	printf("    },\n");
      }
    }
    printf("  ],\n");
  }
  if (driver->shortdescription != NULL) {
    printf("  'shortdescription' => '%s',\n",
	   driver->shortdescription);
  }
  if (driver->locales != NULL) {
    printf("  'locales' => '%s',\n",
	   driver->locales);
  }
  if (driver->num_packages != 0) {
    printf("  'packages' => [\n");
    for (i = 0;
	 i < driver->num_packages; i ++) {
      if (driver->packageurls[i] != 
	  NULL) {
	printf("    {\n");
	printf("      'url' => '%s',\n",
	       driver->packageurls[i]);
	if (driver->packagescopes[i]
	    != NULL)
	  printf("      'scope' => '%s',\n",
		 driver->packagescopes[i]);
	if (driver->packagefingerprints[i]
	    != NULL)
	  printf("      'fingerprint' => '%s',\n",
		 driver->packagefingerprints[i]);
	printf("    },\n");
      }
    }
    printf("  ],\n");
  }
  if (driver->maxresx != NULL) {
    printf("  'drvmaxresx' => '%s',\n",
	   driver->maxresx);
  }
  if (driver->maxresy != NULL) {
    printf("  'drvmaxresy' => '%s',\n",
	   driver->maxresy);
  }
  if (driver->color != NULL) {
    printf("  'drvcolor' => '%s',\n",
	   driver->color);
  }
  if (driver->text != NULL) {
    printf("  'text' => '%s',\n",
	   driver->text);
  }
  if (driver->lineart != NULL) {
    printf("  'lineart' => '%s',\n",
	   driver->lineart);
  }
  if (driver->graphics != NULL) {
    printf("  'graphics' => '%s',\n",
	   driver->graphics);
  }
  if (driver->photo != NULL) {
    printf("  'photo' => '%s',\n",
	   driver->photo);
  }
  if (driver->load != NULL) {
    printf("  'load' => '%s',\n",
	   driver->load);
  }
  if (driver->speed != NULL) {
    printf("  'speed' => '%s',\n",
	   driver->speed);
  }
  if (driver->num_requires != 0) {
    printf("  'requires' => [\n");
    for (i = 0;
	 i < driver->num_requires; i ++) {
      if (driver->requires[i] != 
	  NULL) {
	printf("    {\n");
	printf("      'driver' => '%s',\n",
	       driver->requires[i]);
	if (driver->requiresversion[i]
	    != NULL)
	  printf("      'version' => '%s',\n",
		 driver->requiresversion[i]);
	printf("    },\n");
      }
    }
    printf("  ],\n");
  }
  if (driver->driver_type) {
    printf("  'type' => '%s',\n", driver->driver_type);
  }
  if (driver->cmd) {
    printf("  'cmd' => '%s',\n", driver->cmd);
  }
  if (driver->cmd_pdf) {
    printf("  'cmd_cmd' => '%s',\n", driver->cmd_pdf);
  }
  if (driver->driverppdentry) {
    printf("  'ppdentry' => '%s',\n", driver->driverppdentry);
  } else {
    printf("  'ppdentry' => undef,\n");
  }
  if (driver->drivermargins) {
    printf("  'margins' => {\n");
    generateMarginsPerlData(driver->drivermargins, debug);
    printf("  },\n");
  }
  if (driver->comment) {
    printf("  'comment' => '%s',\n", driver->comment);
  }
  if (driver->num_printers > 0) {
    printf("  'printers' => [\n");
    for (i = 0; i < driver->num_printers; i ++) {
      printf("    {\n");
      printf("      'id' => '%s',\n",
	     driver->printers[i]->id);
      if (driver->printers[i]->comment) {
	printf("      'comment' => '%s'\n",
	       driver->printers[i]->comment);
      }
      if (driver->printers[i]->excmaxresx != NULL) {
	printf("      'excmaxresx' => '%s',\n",
	       driver->printers[i]->excmaxresx);
      }
      if (driver->printers[i]->excmaxresy != NULL) {
	printf("      'excmaxresy' => '%s',\n",
	       driver->printers[i]->excmaxresy);
      }
      if (driver->printers[i]->exccolor != NULL) {
	printf("      'exccolor' => '%s',\n",
	       driver->printers[i]->exccolor);
      }
      if (driver->printers[i]->exctext != NULL) {
	printf("      'exctext' => '%s',\n",
	       driver->printers[i]->exctext);
      }
      if (driver->printers[i]->exclineart != NULL) {
	printf("      'exclineart' => '%s',\n",
	       driver->printers[i]->exclineart);
      }
      if (driver->printers[i]->excgraphics != NULL) {
	printf("      'excgraphics' => '%s',\n",
	       driver->printers[i]->excgraphics);
      }
      if (driver->printers[i]->excphoto != NULL) {
	printf("      'excphoto' => '%s',\n",
	       driver->printers[i]->excphoto);
      }
      if (driver->printers[i]->excload != NULL) {
	printf("      'excload' => '%s',\n",
	       driver->printers[i]->excload);
      }
      if (driver->printers[i]->excspeed != NULL) {
	printf("      'excspeed' => '%s',\n",
	       driver->printers[i]->excspeed);
      }
      printf("    },\n");
    }
    printf("  ]\n");
  } else {
    printf("  'printers' => []\n");
  }
  printf("};\n");

}

int /* O - Error state */
main(int argc, char **argv) { /* I - Command line arguments */
  int i, j; /* loop variables */
  int           debug = 0; /* Debug output level */
  xmlChar       *setting; 
  xmlChar       *language = "C"; 
  xmlChar       **defaultsettings = NULL; /* User-supplied option settings*/
  int           num_defaultsettings = 0;
  char          *filename = NULL;
  int           datatype = 1;  /* Data type to parse: 0: Overview, 1: Combo 
				  2: Printer, 3: Driver */
  comboDataPtr  combo;  /* C data structure of printer/driver combo */
  overviewPtr   overview;  /* C data structure of printer/driver combo */
  printerEntryPtr printer;  /* C data structure of printer entry */
  driverEntryPtr driver;  /* C data structure of driver entry*/

  /* COMPAT: Do not genrate nodes for formatting spaces */
  LIBXML_TEST_VERSION
  xmlKeepBlanksDefault(0);
  
  /* Read the command line arguments */
  for (i = 1; i < argc; i ++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'O' : /* Parse overview */
	datatype = 0;
	break;
      case 'C' : /* Parse combo */
	datatype = 1;
	break;
      case 'P' : /* Parse printer */
	datatype = 2;
	break;
      case 'D' : /* Parse driver */
	datatype = 3;
	break;
      case 'o' : /* option setting */
	if (argv[i][2] != '\0')
	  setting = (xmlChar *)(argv[i] + 2);
	else {
	  i ++;
	  setting = (xmlChar *)(argv[i]);
	}
	num_defaultsettings ++;
	defaultsettings =
	  (xmlChar **)realloc((xmlChar **)defaultsettings, 
			      sizeof(xmlChar *) * num_defaultsettings);
	defaultsettings[num_defaultsettings-1] = strdup(setting);
	break;
      case 'l' : /* language */
	if (argv[i][2] != '\0')
	  language = (xmlChar *)(argv[i] + 2);
	else {
	  i ++;
	  language = (xmlChar *)(argv[i]);
	}
	break;
      case 'v' : /* verbose */
	debug++;
	j = 2;
	while (argv[i][j] != '\0') {
	  if (argv[i][j] == 'v') debug++;
	  j++;
	}
	break;
      case '?' :
      case 'h' : /* Help */
	fprintf(stderr, "Usage: foomatic-perl-data [ -O ] [ -C ] [ -P ] [ -D ]\n                          [ -o option=setting ] [ -o ... ] [ -l language ]\n                          [ -v ] [ -vv ] [ filename ]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   -O           Parse overview XML data\n");
	fprintf(stderr, "   -C           Parse printer/driver combo XML data (default)\n");
	fprintf(stderr, "   -P           Parse printer entry XML data\n");
	fprintf(stderr, "   -D           Parse driver entry XML data\n");
	fprintf(stderr, "   -o option=setting\n");
	fprintf(stderr, "                Default option settings for the generated Perl data (combo\n");
	fprintf(stderr, "                only, no range-checking)\n");
	fprintf(stderr, "   -l language  Language in which the texts are returned, default is \"en\"\n");
	fprintf(stderr, "                (English). If the text in the requested language is missing,\n");
	fprintf(stderr, "                english text will be returned.\n");
	fprintf(stderr, "   -v           Verbose (debug) mode\n");
	fprintf(stderr, "   -vv          Very verbose (debug) mode\n");
	fprintf(stderr, "   filename     Read input from a file and not from standard input\n");
	fprintf(stderr, "\n");
	exit(1);
       	break;
      default :
	fprintf(stderr, "Unknown option \'-%c\'!\n", argv[i][1]);
	exit(1);
      }
    } else {
      filename = argv[i];
    }
  }

  if (debug) fprintf(stderr,"Language: %s\n", language);
  
  if (datatype == 0) { /* Parse overview data */

    /* Parse the XML input */
    overview = parseOverviewFile(filename, language, debug);

    if (overview) {

      /* Generate the Perl data structure on standard output */
      generateOverviewPerlData(overview, debug);

    } else {
      exit(1);
    }

  } else if (datatype == 1) { /* Parse printer/driver combo data */
  
    /* Parse the XML input */
    combo = parseComboFile(filename, language, debug);

    if (combo) {

      /* Prepare the data for the output */
      prepareComboData(combo, defaultsettings, num_defaultsettings, debug);

      /* Generate the Perl data structure on standard output */
      generateComboPerlData(combo, debug);

    } else {
      exit(1);
    }

  } else if (datatype == 2) { /* Parse overview data */

    /* Parse the XML input */
    printer = parsePrinterFile(filename, language, debug);

    if (printer) {

      /* Generate the Perl data structure on standard output */
      generatePrinterPerlData(printer, debug);

    } else {
      exit(1);
    }

  } else if (datatype == 3) { /* Parse overview data */

    /* Parse the XML input */
    driver = parseDriverFile(filename, language, debug);

    if (driver) {

      /* Generate the Perl data structure on standard output */
      generateDriverPerlData(driver, debug);

    } else {
      exit(1);
    }

  }

  /* Clean up everything else before quitting. */
  xmlCleanupParser();
  
  return(0);
}
