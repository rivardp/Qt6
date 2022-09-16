// FileChecks.h		Version 1.0		March 2, 2009
//
/*
#include <sys/stat.h> 

bool FileExists(PString targetFile)
{ 
  struct _stat stFileInfo; 
  bool exists; 
  int iStat; 

  // Attempt to get the file attributes 
  iStat = _wstat(targetFile.getString(),&stFileInfo); 
  if(iStat == 0)
  { 
    // We were able to get the file attributes 
    // so the file obviously exists. 
    exists = true; 
  }
  else
  { 
    // We were not able to get the file attributes. 
    // This may mean that we don't have permission to 
    // access the folder which contains this file. If you 
    // need to do that level of checking, lookup the 
    // return values of stat which will give you 
    // more details on why stat failed. 
    exists = false; 
  } 

  return exists;

}
*/
