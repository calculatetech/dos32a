/***************************************************************************
 *
 * package.h
 *
 * Copyright (c) 1996, Tenberry Software, Inc.
 * All Rights Reserved
 *
 */

#ifndef PACKAGE_H
#define PACKAGE_H

typedef unsigned long ACTION_RETURN;
typedef ACTION_RETURN (far ACTION)();       /* Package actions return ulong */
typedef ACTION_RETURN (far * ACTIONPTR) (); /* Package actions return ulong */
typedef struct action_pack
        {
        char   far *action_name;
        ACTION far *action;
        } ACTION_PACK;

typedef struct package
        {
        struct package far *next_package;  /* Pointer to next package in chain */
        char far *package_title;        /* Optional pointer to ascii name   */
        unsigned char major_rev;        /* Optional revision levels         */
        unsigned char minor_rev;
        int system_action_count;        /* System action count              */
        ACTION_PACK far *system_action; /* Pointer to vector of pre-defined */
        int user_action_count;          /* Optional user count              */
        ACTION_PACK far *user_action;   /* Pointer to vector of user_defined*/
        } PACKAGE;

#define PackageNext(package) package->next_package
#define PackageTitle(package) package->package_title
#define PackageRev(package) package->major_rev * 100 + package->minor_rev
#define PackageMajRev(package) package->major_rev
#define PackageMinRev(package) package->minor_rev

/* From jab */
void far *     RSIAPI D16PkgFindAction (PACKAGE far *p, char far *s);
PACKAGE far *  RSIAPI D16PkgFindPackage (char far *s);
int            RSIAPI D16PkgUnregister (PACKAGE far *p);
void           RSIAPI D16PkgRegister (PACKAGE far *p);
void           RSIAPI D16PkgTsr (int keep);

/* From sean */
void far *     RSIAPI D16FindAction (PACKAGE far *p, char far *s);
PACKAGE far *  RSIAPI D16FindPackage (char far *s);
int            RSIAPI D16PackageUnregister (PACKAGE far *p);
void           RSIAPI D16PackageRegister (PACKAGE far *p);
void           RSIAPI D16PackageTsr (int keep);

#endif
