//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
// Inc. Tripwire is a registered trademark of Tripwire, Inc.  All rights
// reserved.
// 
// This program is free software.  The contents of this file are subject
// to the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.  You may redistribute it and/or modify it
// only in compliance with the GNU General Public License.
// 
// This program is distributed in the hope that it will be useful.
// However, this program is distributed AS-IS WITHOUT ANY
// WARRANTY; INCLUDING THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
// FOR A PARTICULAR PURPOSE.  Please see the GNU General Public License
// for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
// 
// Nothing in the GNU General Public License or any other license to use
// the code or files shall permit you to use Tripwire's trademarks,
// service marks, or other intellectual property without Tripwire's
// prior written consent.
// 
// If you have any questions, please contact Tripwire, Inc. at either
// info@tripwire.org or www.tripwire.org.
//
// hierdatabase_t
#include "db/stddb.h"
#include "db/hierdatabase.h"
#include "test.h"
#include "core/error.h"



static const TSTRING g_block_data = "Hello World Hello World Hello World Hello World";


static void AddFile(cHierDatabase::iterator& iter, const TSTRING& filename, bool with_data=false)
{
    if (iter.SeekTo(filename.c_str()))
        TCOUT << "Object " << filename << " already exists!" << std::endl;

    iter.CreateEntry(filename);

    if (with_data)
    {
        iter.SetData( (int8*)g_block_data.c_str(), g_block_data.length() + 1 );
    }

    TEST(iter.HasData() == with_data);
}

static void AddDirectory(cHierDatabase::iterator& iter, const TSTRING& filename)
{
    if (iter.SeekTo(filename.c_str()))
        TCOUT << "Object " << filename << " already exists!" << std::endl;

    iter.CreateEntry(filename);
    iter.CreateChildArray();

    TEST(iter.CanDescend());
}

static void RemoveDirectory(cHierDatabase::iterator& iter, const TSTRING& filename)
{
    TCOUT << "Removing the child of " << filename << std::endl;
    if( iter.SeekTo( filename.c_str() ) )
    {
        //TODO -- check that it has an empty child
        iter.DeleteChildArray();
        iter.DeleteEntry();
    }
    else
    {
        TCOUT << "Unable to find object " << filename << std::endl;
    }
}

static void RemoveFile(cHierDatabase::iterator& iter, const TSTRING& filename)
{
    TCOUT << "Removing object " << filename << std::endl;
    if( iter.SeekTo( filename.c_str() ) )
    {
        if( iter.CanDescend() )
        {
            TCOUT << "Can't delete object; it still has children." << std::endl;
        }
        else
        {
            iter.RemoveData();
            iter.DeleteEntry();
        }
    }
    else
    {
        TCOUT << "Unable to find object " << filename << std::endl;
    }
}

static void ChDir(cHierDatabase::iterator& iter, const TSTRING& filename)
{
    if( filename.compare( _T("..") ) == 0 )
    {
        if( iter.AtRoot() )
          TCOUT << "At root already" << std::endl;

        TCOUT << "Ascending..." << std::endl;
        iter.Ascend();
    }
    else
    {
      if( iter.SeekTo( filename.c_str() ) )
      {
          if( !iter.CanDescend())
              TCOUT << filename << " has no children; can't descend." << std::endl;

          TCOUT << "Descending into " << filename << std::endl;
          iter.Descend();
      }
      else
      {
          TCOUT << "Unable to find object " << filename << std::endl;
      }
    }
}

static void AssertData(cHierDatabase::iterator& iter, const TSTRING& filename, bool should_have)
{
    bool exists = iter.SeekTo( filename.c_str() );
    TEST(exists == should_have);

    if (exists)
    {
        bool has_data = iter.HasData();
        TEST(has_data == should_have);

        if (has_data)
        {
            int32 dummyLength;
            TSTRING read_str( (TCHAR*)iter.GetData(dummyLength) );
            TEST(read_str == g_block_data);
        }
    }
}

static void AssertExists(cHierDatabase::iterator& iter, const TSTRING& filename, bool should_have)
{
    bool exists = iter.SeekTo( filename.c_str() );
    TEST(exists == should_have);
}

static void AssertChildren(cHierDatabase::iterator& iter, const TSTRING& filename, bool should_have)
{
    bool exists = iter.SeekTo( filename.c_str() );

    if (exists)
    {
        bool has_children = iter.CanDescend();
        TEST(has_children == should_have);
    }
}

void TestHierDatabaseBasic()
{
    cHierDatabase db;
    db.Open( _T("test.db"), 5, true);
    cHierDatabase::iterator iter(&db);

    AddFile(iter, "file1", true);
    AddFile(iter, "file2", false);
    AddFile(iter, "file3", false);

    AddDirectory(iter, "dir1");
    AddDirectory(iter, "dir2");
    AddDirectory(iter, "dir3");

    AssertData(iter, "file1", true);

    ChDir(iter, "dir1");
    AddFile(iter, "dir1_file1");
    ChDir(iter, "..");

    RemoveFile(iter, "file1");
    RemoveFile(iter, "file2");

    AssertExists(iter, "file1", false);
    AssertExists(iter, "file2", false);
    AssertExists(iter, "file3", true);

    RemoveDirectory(iter, "dir2");

    AssertExists(iter, "dir1", true);
    AssertExists(iter, "dir2", false);
    AssertExists(iter, "dir3", true);

    AssertChildren(iter, "dir1", true);
    AssertChildren(iter, "dir3", true);
    AssertChildren(iter, "file3", false);

#ifdef DEBUG
    db.AssertAllBlocksValid();
#endif
}

