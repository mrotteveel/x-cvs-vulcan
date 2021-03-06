
#include "fbdev.h"

#include "../jrd/ibase.h"
#include "../jrd/common.h"
#include <stdio.h>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Usage: %s <new db name>\n", argv[0]);
		return -1;
	}

	UCHAR dpb[128];
	UCHAR* d = dpb;
	*d++ = (UCHAR) isc_dpb_version1;
	*d++ = (UCHAR) isc_dpb_set_db_readonly;
	*d++ = 1;
	*d++ = TRUE;
	const SSHORT len = d - dpb;

	ISC_STATUS_ARRAY sv;
	isc_db_handle db = 0;
	isc_tr_handle tr = 0;


	isc_create_database(sv, 0, argv[1], &db, 0, 0, 0);
	if (sv[0] == 1 && sv[1] > 0)
	{
		isc_print_status(sv);
		return -2;
	}
	isc_start_transaction(sv, &tr, 1, &db, 0, 0);
	if (sv[0] == 1 && sv[1] > 0)
	{
		isc_print_status(sv);
		return -3;
	}
	isc_commit_transaction(sv, &tr);
	if (sv[0] == 1 && sv[1] > 0)
	{
		isc_print_status(sv);
		return -3;
	}
	isc_detach_database(sv, &db);
	if (sv[0] == 1 && sv[1] > 0)
	{
		isc_print_status(sv);
		return -3;
	}
	isc_attach_database(sv, 0, argv[1], &db, len, (char*) dpb);
	if (sv[0] == 1 && sv[1] > 0)
	{
		isc_print_status(sv);
		return -4;
	}
	isc_detach_database(sv, &db);
	if (sv[0] == 1 && sv[1] > 0)
	{
		isc_print_status(sv);
		return -5;
	}

	return 0;
}


