#ifndef __orderinfo_h__
#define __orderinfo_h__

////////////////////////////////////////////////////////////
enum OrdStatus 
{
    status_NEW = 0,
	status_PARTIALLY_FILLED = 1,
	status_FILLED = 2,
	status_DONE_FOR_DAY = 3,
	status_CANCELED = 4,
	status_REPLACED = 5,
	status_PENDING_CANCEL = 6,
	status_STOPPED = 7,
	status_REJECTED = 8,
	status_SUSPENDED = 9,
	status_PENDING_NEW = 10,
	status_CALCULATED = 11,
	status_EXPIRED = 12,
	status_ACCEPTED_FOR_BIDDING = 13,
	status_PENDING_REPLACE = 14,
};

#endif // __orderinfo_h__
