#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include "zlib.h"
const char* SERVER = "http://localhost:7000/";
struct FPC_Load_Status
{
  char* ptr;
};
int crc32_str(const char* ptr)
{
  long crc = crc32(0L, Z_NULL, 0);
  crc = crc32(crc, (const unsigned char*)ptr, strlen(ptr));
  return crc;
}
int FPC_GetFilesize(const char* filename)
{
  char buf[512];
  sprintf(buf,"%s%s",SERVER,filename);
  CURL *curl;
  CURLcode res;
  curl = curl_easy_init();
  double d_filesize;
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl,CURLOPT_HEADER ,1 );
    curl_easy_setopt(curl,CURLOPT_NOBODY ,1 );
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res == CURLE_OK)
    {
      curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d_filesize);
      curl_easy_cleanup(curl);
      return (int)d_filesize;
    }

  }
  return -1;
  //TODO RETURN ORIGINAL FUNCTION;
}
int* FPCCache;
int  FPCCacheSize = 0;
int FPC_DoesServerHave(const char* filename)
{
  if(FPCCacheSize == 0)
  {

    return 1;
  }
  long crc = crc32_str(filename);
  for(int i = 0; i < FPCCacheSize; i++)
  {
    if(FPCCache[i] == crc)
    {
      printf("Found %s in cache! The server has it!\n",filename);
      return 1;
    }
    else
    {
      printf("%X doesn't match %X\n",FPCCache[i],crc);
    }
  }
  printf("Gotta load %s from disk...\n",filename);
  return 0;
}

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  struct FPC_Load_Status *status = (struct FPC_Load_Status*)userp;
  size_t realsize = size * nmemb;
  memcpy(status->ptr,contents,realsize);
  status->ptr += realsize;
  return realsize;
}
int _FPC_LoadFile(const char* filename,char* ptr,int bypassCache)
{
  char buf[512];
  sprintf(buf,"%s%s",SERVER,filename);
  //Do we even have to hit the server?
  if(bypassCache || FPC_DoesServerHave(filename))
  {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, buf);
      /* example.com is redirected, so we tell libcurl to follow redirection */
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
      /* send all data to this function  */
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      struct FPC_Load_Status status;
      status.ptr = ptr;
      /* we pass our 'chunk' struct to the callback function */
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&status);
      /* Perform the request, res will get the return code */
      res = curl_easy_perform(curl);
      /* Check for errors */
      if(res == CURLE_OK)
      {
        curl_easy_cleanup(curl);
        return 1;
      }
    }
  }
  //If we got here, we are using cache and this file isn't on the server...
  //or curl failed somehow...
  printf("curl fail");
  return -1;
  //TODO RETURN ORIGINAL FUNCTION;
}
int FPC_LoadFile(const char* filename,char* ptr)
{
  return _FPC_LoadFile(filename,ptr,0);
}
int FPC_LoadFileUncached(const char* filename,char* ptr)
{
  return _FPC_LoadFile(filename,ptr,1);
}
char* FPC_AllocateLoad(const char* filename)
{
  int filesize = FPC_GetFilesize(filename);
  char* data = malloc(filesize);
  FPC_LoadFile(filename,data);
  return data;
}
void FPC_InitCache()
{
  const char* cachePath = "cache";
  int filesize = FPC_GetFilesize(cachePath);
  char* data = malloc(filesize);
  if(FPC_LoadFileUncached(cachePath,data))
  {
    FPCCache = (int *)data;
    FPCCacheSize = filesize/4;
    printf("Loaded %d entries from cachefile\n",FPCCacheSize);

  }
  else
  {
    printf("Something went wrong loading the cache file\n");
    FPCCacheSize = 0;
  }

}
int main()
{
  FPC_InitCache();
  char* data = FPC_AllocateLoad("test");
  printf("%s",data);
  return 0;
}
