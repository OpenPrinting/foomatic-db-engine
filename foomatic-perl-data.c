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
 * Records for the data of the overview
 */

typedef struct overviewPrinter {
  /* General info */
  xmlChar *id;
  xmlChar *make;
  xmlChar *model;
  xmlChar *functionality;
  xmlChar *unverified;
  /* Printer auto-detection */
  xmlChar *par_mfg;
  xmlChar *par_mdl;
  xmlChar *par_des;
  xmlChar *par_cmd;
  xmlChar *usb_mfg;
  xmlChar *usb_mdl;
  xmlChar *usb_des;
  xmlChar *usb_cmd;
  xmlChar *snmp_mfg;
  xmlChar *snmp_mdl;
  xmlChar *snmp_des;
  xmlChar *snmp_cmd;
  /* Drivers */
  xmlChar *driver;
  int     num_drivers;
  xmlChar **drivers;
} overviewPrinter, *overviewPrinterPtr;
  
typedef struct overview {
  int     num_overviewPrinters;
  overviewPrinterPtr *overviewPrinters;
} overview, *overviewPtr;

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
  xmlChar *spot;
  xmlChar *order;
  xmlChar *section;
  xmlChar *grouppath;
  xmlChar *proto;
  xmlChar *required;
  xmlChar *min_value;
  xmlChar *max_value;
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
  /* Printer properties */
  xmlChar *color;
  xmlChar *ascii;
  xmlChar *pjl;
  /* Printer auto-detection */
  xmlChar *par_mfg;
  xmlChar *par_mdl;
  xmlChar *par_des;
  xmlChar *par_cmd;
  xmlChar *usb_mfg;
  xmlChar *usb_mdl;
  xmlChar *usb_des;
  xmlChar *usb_cmd;
  xmlChar *snmp_mfg;
  xmlChar *snmp_mdl;
  xmlChar *snmp_des;
  xmlChar *snmp_cmd;
  /* Driver */
  xmlChar *driver;
  xmlChar *pcdriver;
  xmlChar *driver_type;
  xmlChar *driver_comment;
  xmlChar *url;
  xmlChar *cmd;
  xmlChar *nopjl;
  /* Driver options */
  int     num_args;
  argPtr  *args;
  xmlChar *maxspot;
} comboData, *comboDataPtr;

/*
 * Record for a Foomatic printer entry
 */

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
  /* Printer auto-detection */
  xmlChar *par_mfg;
  xmlChar *par_mdl;
  xmlChar *par_des;
  xmlChar *par_cmd;
  xmlChar *usb_mfg;
  xmlChar *usb_mdl;
  xmlChar *usb_des;
  xmlChar *usb_cmd;
  xmlChar *snmp_mfg;
  xmlChar *snmp_mdl;
  xmlChar *snmp_des;
  xmlChar *snmp_cmd;
  /* Misc items */
  xmlChar *functionality;
  xmlChar *driver;
  xmlChar *unverified;
  xmlChar *url;
  xmlChar *contriburl;
  xmlChar *comment;
} printerEntry, *printerEntryPtr;
  
/*
 * Records for a Foomatic driver entry
 */

typedef struct drvPrnEntry {
  xmlChar *id;
  xmlChar *comment;
} drvPrnEntry, *drvPrnEntryPtr;

typedef struct driverEntry {
  xmlChar *id;
  xmlChar *name;
  xmlChar *url;
  xmlChar *driver_type;
  xmlChar *cmd;
  xmlChar *comment;
  int     num_printers;
  drvPrnEntryPtr *printers;
} driverEntry, *driverEntryPtr;

/*
 * Function to quote "'" and "\" in a string
 */

static xmlChar* /* O - String with quoted "'" */
perlquote(xmlChar *str) { /* I - Original string */
  xmlChar *dest, *s;
  int offset = 0;

  dest = xmlStrdup(str);
  while (((const xmlChar *)s = 
	  xmlStrchr((const xmlChar *)(dest + offset), '\'')) ||
	 ((const xmlChar *)s = 
	  xmlStrchr((const xmlChar *)(dest + offset), '\\'))) {
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
  xmlChar        *id;  /* Full printer ID, with "printer/" */
  xmlChar        *charset;
  overviewPrinterPtr printer;

  /* Allocate memory for the option */
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
  printer->functionality = NULL;
  printer->unverified = NULL;
  printer->par_mfg = NULL;
  printer->par_mdl = NULL;
  printer->par_des = NULL;
  printer->par_cmd = NULL;
  printer->usb_mfg = NULL;
  printer->usb_mdl = NULL;
  printer->usb_des = NULL;
  printer->usb_cmd = NULL;
  printer->snmp_mfg = NULL;
  printer->snmp_mdl = NULL;
  printer->snmp_des = NULL;
  printer->snmp_cmd = NULL;
  printer->driver = NULL;
  printer->num_drivers = 0;
  printer->drivers = NULL;

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
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "driver"))) {
      printer->driver = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Recommended driver: %s\n",
			 printer->driver);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "drivers"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "driver"))) {
	  printer->num_drivers ++;
	  printer->drivers =
	    (xmlChar **)realloc((xmlChar **)printer->drivers, 
				sizeof(xmlChar *) * printer->num_drivers);
	  printer->drivers[printer->num_drivers-1] =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Printer works with: %s\n",
			     printer->drivers[printer->num_drivers-1]);
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "autodetect"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "parallel"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (parallel port):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
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
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
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
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
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
  xmlChar        *id;  /* Full printer ID, with "printer/" */
  xmlChar        *charset;

  /* Initialization of entries */
  ret->id = NULL;
  ret->make = NULL;
  ret->model = NULL;
  ret->pcmodel = NULL;
  ret->color = (xmlChar *)"0";
  ret->pjl = (xmlChar *)"undef";
  ret->ascii = (xmlChar *)"0";
  ret->par_mfg = NULL;
  ret->par_mdl = NULL;
  ret->par_des = NULL;
  ret->par_cmd = NULL;
  ret->usb_mfg = NULL;
  ret->usb_mdl = NULL;
  ret->usb_des = NULL;
  ret->usb_cmd = NULL;
  ret->snmp_mfg = NULL;
  ret->snmp_mdl = NULL;
  ret->snmp_des = NULL;
  ret->snmp_cmd = NULL;

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
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "pcmodel"))) {
      ret->pcmodel = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Model part for PC filename in PPD: %s\n", ret->pcmodel);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "mechanism"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "color"))) {
	  ret->color = (xmlChar *)"1";
	  if (debug) fprintf(stderr, "  Color printer\n");
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
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "parallel"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (parallel port):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
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
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
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
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
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
  xmlChar        *id;  /* Full driver ID, with "driver/" */

  /* Initialization of entries */
  ret->driver = NULL;
  ret->driver_type = NULL;
  ret->driver_comment = NULL;
  ret->url = NULL;
  ret->cmd = NULL;
  ret->nopjl = (xmlChar *)"0";

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
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "pcdriver"))) {
      ret->pcdriver = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver part of PC file name in PPD: %s\n", ret->pcdriver);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "url"))) {
      ret->url = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver URL: %s\n", ret->url);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "comments"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	/*if ((!xmlStrcmp(cur2->name, (const xmlChar *) language))) {*/
	if ((!xmlStrcmp(cur2->name, language))) {
	  ret->driver_comment =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Driver Comment (%s):\n\n%s\n\n",
			     language, ret->driver_comment);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "en"))) {
	  if (!ret->driver_comment) {
	    ret->driver_comment =
	      perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode,
					     1));
	    if (debug) fprintf(stderr, "  Driver Comment (en):\n\n%s\n\n",
			       ret->driver_comment);
	  }
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "execution"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ghostscript"))) {
	  ret->driver_type = (xmlChar *)"G";
	  if (debug) fprintf(stderr, "  Driver type: GhostScript\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "filter"))) {
	  ret->driver_type = (xmlChar *)"F";
	  if (debug) fprintf(stderr, "  Driver type: Filter\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "uniprint"))) {
	  ret->driver_type = (xmlChar *)"U";
	  if (debug) fprintf(stderr, "  Driver type: Uniprint\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "postscript"))) {
	  ret->driver_type = (xmlChar *)"P";
	  if (debug) fprintf(stderr, "  Driver type: PostScript\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "nopjl"))) {
	  ret->nopjl = (xmlChar *)"1";
	  if (debug) fprintf(stderr, "  Driver suppresses PJL options\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "prototype"))) {
	  ret->cmd =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Driver command line:\n\n    %s\n\n",
			     ret->cmd);
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
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "en"))) {
	      enum_val->value =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "      Choice short name: %s\n",
				 enum_val->value);
	    }
	    cur3 = cur3->next;
	  } 
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ev_longname"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) language))) {
	      enum_val->comment =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,"      Choice long name (%s): %s\n",
				 language, enum_val->comment);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "en"))) {
	      if (!enum_val->comment) {
		enum_val->comment =
		  perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
						 1));
		if (debug) fprintf(stderr,
				   "      Choice long name (en): %s\n",
				   enum_val->comment);
	      }
	    }
	    cur3 = cur3->next;
	  }
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
      option->spot = NULL;
      option->order = NULL;
      option->section = NULL;
      option->grouppath = NULL;
      option->proto = NULL;
      option->required = NULL;
      option->min_value = NULL;
      option->max_value = NULL;
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
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "en"))) {
	      option->name =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    Option short name: %s\n",
				 option->name);
	    }
	    cur3 = cur3->next;
	  } 
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_shortname_false"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "en"))) {
	      option->name_false =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr,
				 "    Option short name if false: %s\n",
				 option->name_false);
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_longname"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) language))) {
	      option->comment =
		perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
					       1));
	      if (debug) fprintf(stderr, "    Option long name (%s): %s\n",
				 language, option->comment);
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "en"))) {
	      if (!option->comment) {
		option->comment =
		  perlquote(xmlNodeListGetString(doc, cur3->xmlChildrenNode,
						 1));
		if (debug) fprintf(stderr,
				   "    Option long name (en): %s\n",
				   option->comment);
	      }
	    }
	    cur3 = cur3->next;
	  }
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "arg_execution"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_substitution"))) {
	      option->style = (xmlChar *)"C";
	      if (debug)
		fprintf(stderr,
			"    Option style: Command line Substitution\n");
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_postscript"))) {
	      option->style = (xmlChar *)"G";
	      if (debug)
		fprintf(stderr,
			"    Option style: PostScript code\n");
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_pjl"))) {
	      option->style = (xmlChar *)"J";
	      if (debug)
		fprintf(stderr,
			"    Option style: PJL command\n");
	    } else if ((!xmlStrcmp(cur3->name, (const xmlChar *) "arg_collective"))) {
	      option->style = (xmlChar *)"X";
	      if (debug)
		fprintf(stderr,
			"    Option style: Collective option\n");
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

  /* Initialization of entries */
  ret->id = NULL;
  ret->make = NULL;
  ret->model = NULL;
  ret->printer_type = NULL;
  ret->color = (xmlChar *)"0"; 
  ret->maxxres = NULL;
  ret->maxyres = NULL;
  ret->refill = NULL;
  ret->ascii = NULL;
  ret->pjl = (xmlChar *)"0";
  ret->par_mfg = NULL;
  ret->par_mdl = NULL;
  ret->par_des = NULL;
  ret->par_cmd = NULL;
  ret->usb_mfg = NULL;
  ret->usb_mdl = NULL;
  ret->usb_des = NULL;
  ret->usb_cmd = NULL;
  ret->snmp_mfg = NULL;
  ret->snmp_mdl = NULL;
  ret->snmp_des = NULL;
  ret->snmp_cmd = NULL;
  ret->functionality = NULL;
  ret->driver = NULL;
  ret->unverified = (xmlChar *)"0";
  ret->url = NULL;
  ret->contriburl = NULL;
  ret->comment = NULL;

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
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "consumables"))) {
	  cur3 = cur2->xmlChildrenNode;
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "comments"))) {
	      cur4 = cur3->xmlChildrenNode;
	      while (cur4 != NULL) {
		if ((!xmlStrcmp(cur4->name, (const xmlChar *) language))) {
		  ret->refill = 
		    perlquote(xmlNodeListGetString(doc,
						   cur4->xmlChildrenNode,
						   1));
		  if (debug) fprintf(stderr,
				     "  Consumables (%s): %s\n",
				     language, ret->refill);
		  
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "en"))) {
		  if (!ret->refill) {
		    ret->refill = 
		      perlquote(xmlNodeListGetString(doc,
						     cur4->xmlChildrenNode,
						     1));
		    if (debug) fprintf(stderr,
				       "  Consumables (en): %s\n",
				       ret->refill);
		  }
		}
		cur4 = cur4->next;
	      }
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
	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "autodetect"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "parallel"))) {
	  cur3 = cur2->xmlChildrenNode;
	  if (debug) fprintf(stderr, "  Printer auto-detection info (parallel port):\n");
	  while (cur3 != NULL) {
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
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
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
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
	    if ((!xmlStrcmp(cur3->name, (const xmlChar *) "manufacturer"))) {
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
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "unverified"))) {
      ret->unverified = (xmlChar *)"1";
      if (debug) fprintf(stderr, "  Printer entry is unverified\n");
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
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) language))) {
	  ret->comment =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Comment (%s):\n\n%s\n\n",
			     language, ret->comment);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "en"))) {
	  if (!ret->comment) {
	    ret->comment =
	      perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	    if (debug) fprintf(stderr, "  Comment (en):\n\n%s\n\n",
			     ret->comment);
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

  /* Initialization of entries */
  ret->id = NULL;
  ret->name = NULL;
  ret->url = NULL;
  ret->driver_type = NULL;
  ret->cmd = NULL;
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
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "url"))) {
      ret->url = 
	perlquote(xmlNodeListGetString(doc, cur1->xmlChildrenNode, 1));
      if (debug) fprintf(stderr, "  Driver URL: %s\n", ret->url);
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "execution"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) "ghostscript"))) {
	  ret->driver_type = (xmlChar *)"G";
	  if (debug) fprintf(stderr, "  Driver type: GhostScript\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "filter"))) {
	  ret->driver_type = (xmlChar *)"F";
	  if (debug) fprintf(stderr, "  Driver type: Filter\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "uniprint"))) {
	  ret->driver_type = (xmlChar *)"U";
	  if (debug) fprintf(stderr, "  Driver type: Uniprint\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "postscript"))) {
	  ret->driver_type = (xmlChar *)"P";
	  if (debug) fprintf(stderr, "  Driver type: PostScript\n");
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "prototype"))) {
	  ret->cmd =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Driver command line:\n\n    %s\n\n",
			     ret->cmd);
     	}
	cur2 = cur2->next;
      }
    } else if ((!xmlStrcmp(cur1->name, (const xmlChar *) "comments"))) {
      cur2 = cur1->xmlChildrenNode;
      while (cur2 != NULL) {
	if ((!xmlStrcmp(cur2->name, (const xmlChar *) language))) {
	  ret->comment =
	    perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	  if (debug) fprintf(stderr, "  Comment (%s):\n\n%s\n\n",
			     language, ret->comment);
	} else if ((!xmlStrcmp(cur2->name, (const xmlChar *) "en"))) {
	  if (!ret->comment) {
	    ret->comment =
	      perlquote(xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1));
	    if (debug) fprintf(stderr, "  Comment (en):\n\n%s\n\n",
			       ret->comment);
	  }
	}
	cur2 = cur2->next;
      }
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
	      cur4 = cur3->xmlChildrenNode;
	      while (cur4 != NULL) {
		if ((!xmlStrcmp(cur4->name, (const xmlChar *) language))) {
		  entry->comment =
		    perlquote(xmlNodeListGetString(doc, 
						   cur4->xmlChildrenNode,
						   1));
		  if (debug) fprintf(stderr, "    Comment (%s): \n%s\n\n",
				     language, entry->comment);
		} else if ((!xmlStrcmp(cur4->name, (const xmlChar *) "en"))) {
		  if (!entry->comment) {
		    entry->comment =
		      perlquote(xmlNodeListGetString(doc, 
						     cur4->xmlChildrenNode,
						     1));
		    if (debug) fprintf(stderr, "    Comment (en): \n%s\n\n",
				       entry->comment);
		  }
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
    if (!xmlStrcmp(cur->name, (const xmlChar *) "printer")) {
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

  int i, j; /* loop variables */
  overviewPrinterPtr printer;
  
  printf("$VAR1 = [\n");
  for (i = 0; i < overview->num_overviewPrinters; i ++) {
    printer = overview->overviewPrinters[i];
    printf("          {\n");
    printf("            'id' => '%s',\n", printer->id);
    printf("            'make' => '%s',\n", printer->make);
    printf("            'model' => '%s',\n", printer->model);
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
    if (printer->driver) {
      printf("            'driver' => '%s',\n", printer->driver);
    }
    if (printer->num_drivers > 0) {
      printf("            'drivers' => [\n");
      for (j = 0; j < printer->num_drivers; j ++) {
	printf("                           '%s',\n",
	       printer->drivers[j]);
      }
      printf("                         ]\n");
    } else {
      printf("            'drivers' => []\n");
    }
    printf("          },\n");
  }
  printf("        ];\n");

}

void
generateComboPerlData(comboDataPtr combo, /* I/O - Foomatic combo data
					     parsed from XML input */
		      int debug) { /* Debug flag */

  int i, j; /* loop variables */
  
  printf("$VAR1 = {\n");
  printf("  'id' => '%s',\n", combo->id);
  printf("  'make' => '%s',\n", combo->make);
  printf("  'model' => '%s',\n", combo->model);
  if (combo->pcmodel) {
    printf("  'pcmodel' => '%s',\n", combo->pcmodel);
  } else {
    printf("  'pcmodel' => undef,\n");
  }
  printf("  'color' => %s,\n", combo->color);
  printf("  'ascii' => %s,\n", combo->ascii);
  printf("  'pjl' => %s,\n", combo->pjl);
  if (combo->par_mfg) {
    printf("  'pnp_mfg' => '%s',\n", combo->par_mfg);
    printf("  'par_mfg' => '%s',\n", combo->par_mfg);
  } else {
    printf("  'pnp_mfg' => undef,\n");
    printf("  'par_mfg' => undef,\n");
  }
  if (combo->par_mdl) {
    printf("  'pnp_mdl' => '%s',\n", combo->par_mdl);
    printf("  'par_mdl' => '%s',\n", combo->par_mdl);
  } else {
    printf("  'pnp_mdl' => undef,\n");
    printf("  'par_mdl' => undef,\n");
  }
  if (combo->par_des) {
    printf("  'pnp_des' => '%s',\n", combo->par_des);
    printf("  'par_des' => '%s',\n", combo->par_des);
  } else {
    printf("  'pnp_des' => undef,\n");
    printf("  'par_des' => undef,\n");
  }
  if (combo->par_cmd) {
    printf("  'pnp_cmd' => '%s',\n", combo->par_cmd);
    printf("  'par_cmd' => '%s',\n", combo->par_cmd);
  } else {
    printf("  'pnp_cmd' => undef,\n");
    printf("  'par_cmd' => undef,\n");
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
  if (combo->cmd) {
    printf("  'cmd' => '%s',\n", combo->cmd);
  } else {
    printf("  'cmd' => undef,\n");
  }
  if (combo->nopjl) {
    printf("  'drivernopjl' => %s,\n", combo->nopjl);
  } else {
    printf("  'drivernopjl' => 0,\n");
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
    if (combo->args[i]->default_value) {
      printf("      'default' => '%s',\n", combo->args[i]->default_value);
    }
    if (combo->args[i]->num_choices > 0) {
      printf("      'vals_byname' => {\n");
      for (j = 0; j < combo->args[i]->num_choices; j ++) {
	printf("        '%s' => {\n", combo->args[i]->choices[j]->value);
	printf("          'value' => '%s',\n", 
	       combo->args[i]->choices[j]->value);
	printf("          'comment' => '%s',\n",
	       combo->args[i]->choices[j]->comment);
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
  if (printer->refill) {
    printf("  'refill' => '%s',\n", printer->refill);
  }
  if (printer->ascii) {
    printf("  'ascii' => '%s',\n", printer->ascii);
  }
  if (printer->pjl) {
    printf("  'pjl' => '%s',\n", printer->pjl);
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
  if (printer->unverified) {
    printf("  'unverified' => '%s',\n", printer->unverified);
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
  if (driver->url) {
    printf("  'url' => '%s',\n", driver->url);
  }
  if (driver->driver_type) {
    printf("  'type' => '%s',\n", driver->driver_type);
  }
  if (driver->cmd) {
    printf("  'cmd' => '%s',\n", driver->cmd);
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
  xmlChar       *language = "en"; 
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
      case 'v' : /* verbose */
	debug++;
	j = 2;
	while (argv[i][j] != '\0') {
	  if (argv[i][j] == 'v') debug++;
	}
	break;
      case '?' :
      case 'h' : /* Help */
	fprintf(stderr, "Usage: foomatic-perl-data [ -O ] [ -C ] [ -P ] [ -D ]\n                          [ -o option=setting ] [ -o ... ] [ -l language ]\n                          [ -v ] [ filename ]\n");
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
