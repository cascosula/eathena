// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////

#include "basefile.h"

#include "eastorage.h"

#include "eaparser.h"
#include "eacompiler.h"
#include "eaprogram.h"
#include "eadefine.h"
#include "eainstance.h"
#include "eaengine.h"




scriptfile::storage scriptfile::stor;

/// load a single file.
bool scriptfile::load_file(const basics::string<>& filename)
{
	eacompiler compiler;
	return compiler.load_file(filename, 0);
}

/// load list of file.
bool scriptfile::load_file(const basics::vector< basics::string<> >& namelist)
{
	eacompiler compiler;
	basics::vector< basics::string<> >::iterator iter(namelist);
	bool ok = true;
	for(; ok && iter; ++iter)
	{
		ok = compiler.load_file(*iter,0);
	}
	return ok;
}








bool scriptfile::storage::reload()
{
	scriptfile_list::iterator iter(this->files);
	for(; iter; ++iter)
	{
		if( !iter->data->load() )
			return false;
	}
	return true;
}
bool scriptfile::storage::erase(const basics::string<>& filename)
{
	scriptfile_list::data_type* ptr = this->files.search(filename);
	if(ptr)
	{
		this->files.erase(filename);
		delete ptr;
	}
	return true;
}

scriptfile::scriptfile_ptr scriptfile::storage::get_scriptfile(const basics::string<>& filename) const
{
	const scriptfile_ptr* ptr = this->files.search(filename);
	return ptr?*ptr:scriptfile_ptr();
}

scriptfile::scriptfile_ptr scriptfile::storage::create(const basics::string<>& filename)
{
	scriptfile_ptr& obj = this->files[filename];
	*obj = filename;
	return obj;
}

void scriptfile::storage::info() const
{
	scriptfile_list::iterator iter(this->files);
	size_t cnt=0;
	for(; iter; ++iter)
	{
		printf("%s: %i scripts\n", 
			(const char*)(iter->key),
			(int)iter->data->scripts.size());
		cnt += iter->data->scripts.size();
	}
	printf("%i files, %i scripts\n", (int)this->files.size(), (int)cnt);
}











///////////////////////////////////////////////////////////////////////////////
// scriptfile.


///////////////////////////////////////////////////////////////////////////
// get definitions
void scriptfile::get_defines(scriptdefines& defs)
{
	defs += this->definitions;
	// get definitions from parents
	scriptfile_list::iterator iter(this->parents);
	for(; iter; ++iter)
	{
		scriptfile::scriptfile_ptr ptr = scriptfile::stor.get_scriptfile(*iter);
		if( ptr.exists() )
		{
			ptr->get_defines(defs);
		}
	}
}
///////////////////////////////////////////////////////////////////////////
// (forced) loading/reloading of this file.
bool scriptfile::load(bool forced, basics::TObjPtr<eacompiler> compiler)
{
	if( this->is_modified() || forced )
	{	
		this->parents.clear();

		if( compiler->load_file(*this, 0) )
			return false;

		// reload depending files
		scriptfile_list::iterator iter(this->childs);
		for(; iter; ++iter)
		{
			scriptfile_ptr ptr = this->get_scriptfile(*iter);
			if( !ptr.exists() || !ptr->load(true, compiler) )
				return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////
// checking file state and updating the locals at the same time
bool scriptfile::is_modified()
{
	struct stat s;
	return ( 0==stat(this->c_str(), &s) && s.st_mtime!=this->modtime && (this->modtime=s.st_mtime)==this->modtime );
}

