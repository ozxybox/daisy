#pragma once

#include "dy_netdb.h"
#include "dy_brush.h"
#include "xstring.h"

class dy_ot_operator;

dy_ot_operator* dy_ot_create_operator(xstr_t name);



class dy_ot_operator
{
public:
	virtual size_t size()            = 0;
	virtual void   apply(void* data) = 0;

	virtual void   commit()          = 0;
	virtual void   revert()          = 0;
};


// Delete //
struct dy_ot_data_delete
{
	dy_netdb_id target;
};
class dy_ot_operator_delete : public dy_ot_operator
{
public:
	virtual size_t size() { return sizeof(dy_ot_data_delete); };


	// Inherited via dy_ot_operator
	virtual void apply(void* data) override;

	virtual void commit() override;

	virtual void revert() override;

};

// Create plane //
struct dy_ot_data_create_plane
{
	dy_netdb_id parent;
	dy_bplane plane;
};
class dy_ot_operator_create_plane : public dy_ot_operator
{
public:
	virtual size_t size() { return sizeof(dy_ot_data_create_plane); };


	// Inherited via dy_ot_operator
	virtual void apply(void* data) override;

	virtual void commit() override;

	virtual void revert() override;

};

// Create solid //
struct dy_ot_data_create_solid
{
	dy_netdb_id parent;
};
class dy_ot_operator_create_solid : public dy_ot_operator
{
public:
	virtual size_t size() { return sizeof(dy_ot_data_create_solid); };


	// Inherited via dy_ot_operator
	virtual void apply(void* data) override;

	virtual void commit() override;

	virtual void revert() override;

};

// Update plane //
struct dy_ot_data_update_plane
{
	dy_netdb_id target;
	dy_bplane content;
};
class dy_ot_operator_update_plane : public dy_ot_operator
{
public:
	virtual size_t size() { return sizeof(dy_ot_data_update_plane); };


	// Inherited via dy_ot_operator
	virtual void apply(void* data) override;

	virtual void commit() override;

	virtual void revert() override;

};


