/**************************************
 *  File:            .\storage.cpp
 *  Generated by:    c:\ocs\cppddl\cppddl -I. -I..\Workbench .\storage.ssc 
 *  Version:         T1.0D Beta Version
 *  Date:            Mon Jul 05 13:55:04 2004
 **************************************/
#define O(s,f) ((int)&(((s*)0)->f))
#define RELEASE	return 1
typedef void (*SCCBFN)(void*,int,void*);
typedef void (*SCFN)(SCCBFN,void*);
typedef struct {
    int   runtime_version;
    char  *name;
    char  *license;
    int   major_version;
    int   minor_version;
    int   field_ids;
    int   class_ids;
    short *stuff;
    long  *bits;
    int   (*fn)(int OCS_which, void *p);
    SCFN  objects;
    void (*save)(void (*OCS_fn)(void*, ...), void *ocs_handle, int OCS_which, void *OCS_obj);
    int  (*restore)(int OCS_which, void *OCS_obj, int ocs_type, void *OCS_value, int OCS_identifier);
    void *(*create)(int OCS_which);
    int   class_base;
    } SCHEMA;

#define OCS_storage_Scripts	45
#define OCS_storage_TestEnv	44
#define OCS_storage_Script	43
#define OCS_storage_CString	42
#define OCS_storage_ApplicationObject	41
#define OCS_storage_CEditClient	40

/* Declarations for environment MSVC32 */

#ifdef _WIN32

#include <afx.h>
#include <afxwin.h>
#include "TestEnv.h"
#include "Script.h"
#include "Scripts.h"
#include "LinkedList.h"

/* Storage schema "storage" Version 1.2 */

static short OCS_storage_stuff [] = {


  /* Fields */

  /*0*/ 35, 40, O(class ApplicationObject,terminal), 0, 0,
  /*1*/ 35, 41, O(class ApplicationObject,populated), 0, 0,
  /*2*/ 35, -1, -1, 0, 0,
  /*3*/ 10, 36, O(class ApplicationObject,options), 0, 0,
  /*4*/ 35, -1, -1, 0, 0,
  /*5*/ 35, -1, -1, 0, 0,
  /*6*/ -41, 16, O(class ApplicationObject,parent), 0, 0,
  /*7*/ 0, 2, -1, 0, 0,
  /*8*/ 42, 12, O(class ApplicationObject,typeName), 0, 0,
  /*9*/ 42, 8, O(class ApplicationObject,name), 0, 0,
  /*10*/ 0, 3, -1, 0, 0,
  /*11*/ 42, 56, O(class Script,body), 0, 0,
  /*12*/ -45, 56, O(class TestEnv,scripts), 0, 0,
  /*13*/ 0, 1, -1, 0, 0,

  /* Classes */

  /*40*/  sizeof (class CEditClient), -1, 0, -1, 0,	/* CEditClient	*/

  /*41*/  sizeof (class ApplicationObject), 0, 0, 12, 0,	/* ApplicationObject	*/

  /*42*/  sizeof (class CString), -1, 0, 21, 0,	/* CString	*/

  /*43*/  sizeof (class Script), 3, 0, 23, 0,	/* Script	*/
  /*44*/  sizeof (class TestEnv), 6, 0, 33, 0,	/* TestEnv	*/
  /*45*/  sizeof (class Scripts), 9, 0, 43, 0,	/* Scripts	*/

  /* Sub-classes */

 0, 0,         // ApplicationObject.CEditClient
 -1,
 1, 0,         // Script.ApplicationObject
 -1,
 1, 0,         // TestEnv.ApplicationObject
 -1,
 1, 0,         // Scripts.ApplicationObject
 -1,


  /* Field lists */


 10,	/* ApplicationObject.name */
 9,	/* ApplicationObject.typeName */
 8,	/* ApplicationObject.children */
 7,	/* ApplicationObject.parent */
 4,	/* ApplicationObject.options */
 2,	/* ApplicationObject.populated */
 1,	/* ApplicationObject.terminal */
 14,	/* ApplicationObject.wasOpen */
 0,

 11,	/* CString.string */
 0,


 10,	/* ApplicationObject.name */
 9,	/* ApplicationObject.typeName */
 8,	/* ApplicationObject.children */
 7,	/* ApplicationObject.parent */
 4,	/* ApplicationObject.options */
 2,	/* ApplicationObject.populated */
 1,	/* ApplicationObject.terminal */
 14,	/* ApplicationObject.wasOpen */

 12,	/* Script.body */
 0,


 10,	/* ApplicationObject.name */
 9,	/* ApplicationObject.typeName */
 8,	/* ApplicationObject.children */
 7,	/* ApplicationObject.parent */
 4,	/* ApplicationObject.options */
 2,	/* ApplicationObject.populated */
 1,	/* ApplicationObject.terminal */
 14,	/* ApplicationObject.wasOpen */

 13,	/* TestEnv.scripts */
 0,


 10,	/* ApplicationObject.name */
 9,	/* ApplicationObject.typeName */
 8,	/* ApplicationObject.children */
 7,	/* ApplicationObject.parent */
 4,	/* ApplicationObject.options */
 2,	/* ApplicationObject.populated */
 1,	/* ApplicationObject.terminal */
 14,	/* ApplicationObject.wasOpen */

 0,
 0};


static long storage_bits [] = {
 0x0,	/* CEditClient */
 0x23ff,	/* ApplicationObject */
 0x400,	/* CString */
 0x2bff,	/* Script */
 0x33ff,	/* TestEnv */
 0x23ff,	/* Scripts */
 0 };

static void storage_objects (SCCBFN fn, void *stuff)
    {
    CEditClient	ocs_0;
    ApplicationObject	ocs_1;
    Script	ocs_3;
    TestEnv	ocs_4;
    Scripts	ocs_5;

    (fn)(stuff, 0, &ocs_0);
    (fn)(stuff, 1, &ocs_1);
    (fn)(stuff, 3, &ocs_3);
    (fn)(stuff, 4, &ocs_4);
    (fn)(stuff, 5, &ocs_5);
    }

static void *storage_create (int OCS_which)
    {
    switch (OCS_which)
        {
        case 0: return new CEditClient;
        case 1: return new ApplicationObject;
        case 2: return new CString;
        case 3: return new Script;
        case 4: return new TestEnv;
        case 5: return new Scripts;
      }
    return 0;
    }


static void storage_save (void (*OCS_fn)(void*, ...), void *ocs_handle, int OCS_which, void *OCS_obj)
    {
    switch (OCS_which)
        {
        case 3:
             {
             class CString *OCS_object;
             OCS_object = (class CString*) OCS_obj;
             (OCS_fn) (ocs_handle, -1,0,(const char*) *OCS_object);

             }
             break;
        case 2:
             {
             class ApplicationObject *OCS_object;
             OCS_object = (class ApplicationObject*) OCS_obj;
             {
             for (LinkedList *pos = (&OCS_object->children)->getHead (); (&OCS_object->children)->more (pos);)
                 {
                 ApplicationObject *object = (ApplicationObject
*) (&OCS_object->children)->getNext ((&pos));
                 (OCS_fn) (ocs_handle, -41,0,(ApplicationObject
*) object);
                 }
             }
             }
             break;
        case 1:
             {
             class ApplicationObject *OCS_object;
             OCS_object = (class ApplicationObject*) OCS_obj;
             (OCS_fn) (ocs_handle, 35,0,(bool) OCS_object->isOpen ());

             }
             break;
        }
    }

static int storage_restore (int OCS_which, void *OCS_obj, int ocs_type, void *OCS_value, int OCS_identifier)
    {
    switch (OCS_which)
        {
        case 3:
             {
             class CString *OCS_object;
             OCS_object = (class CString*) OCS_obj;
             {
             *OCS_object = (const char*) OCS_value;
             RELEASE;
             }
             }
             break;
        case 2:
             {
             class ApplicationObject *OCS_object;
             OCS_object = (class ApplicationObject*) OCS_obj;
             OCS_object->addChild ((ApplicationObject*) OCS_value);
             }
             break;
        case 1:
             {
             class ApplicationObject *OCS_object;
             OCS_object = (class ApplicationObject*) OCS_obj;
             OCS_object->wasOpen = OCS_value!=0;
             }
             break;
        }
    return 0;
    }

static SCHEMA schema_storage = {
 7, "storage", "Lic001", 1, 2, 14, 6, OCS_storage_stuff, storage_bits,
 0, storage_objects, storage_save, storage_restore, storage_create,
 40,};
#ifdef __cplusplus
extern "C" {
#endif

SCHEMA *OCS_schema_storage_v1 () { return &schema_storage; }
#ifdef __cplusplus
}
#endif

#endif /* MSVC32 */

