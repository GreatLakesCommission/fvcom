#include "util.h"

#ifdef _WIN32
static const char NEWLINE = '\r';
static const char DIRECTORY_SEPARATOR = '\\';
#else
static const char NEWLINE = '\n';
static const char DIRECTORY_SEPARATOR = '/';
#endif
static const char NULCHAR = '\0';

static const int MAXERRMSG = 256;

static const int8_t SHIFT = 8;

struct _hash_node
{
    char *key;
    void *data;
    uint64_t datasize;
    struct _hash_node *next;
};
struct _hash_table
{
    struct _hash_node* bucks[NBUCKETS];
    uint64_t size;
};

//#define hash(H,C) while(*(C)!=NEWLINE&&*(C)!=NULCHAR)(H)=((256*(H)+(*(C)++))%(NBUCKETS));

static void _printErr(const char *fmt,va_list ap,int errNoFlag)
{
    char buf[MAXERRMSG];
    int errno_save = errno;
    vsnprintf(buf,MAXERRMSG,fmt,ap);
    int blen = strlen(buf);
    if(errNoFlag)
        blen += snprintf(buf + blen,MAXERRMSG - blen,": %s",strerror(errno_save));
    if(blen < MAXERRMSG - 1)
        strcat(buf,"\n");/*strcat will add terminating null byte for us, make sure we have space for it*/
    fflush(stdout);
    fputs(buf,stderr);
    fflush(stderr);
}
void printError(int errNoFlag,const char *fmt,...)
{
    va_list ap;
    va_start(ap,fmt);
    _printErr(fmt,ap,errNoFlag);
    va_end(ap);
}

static inline uint64_t hash(const char *key)
{
    uint64_t code = 0;
    while(NULCHAR != *(key) && NEWLINE != *(key))
        code = (code << SHIFT) + *(key++);
    return code % NBUCKETS;
}
static inline void delNode(struct _hash_node *pnode)
{
    if(pnode)
    {
        free(pnode->data);
        free(pnode->key);
        free(pnode);
    }
}
static inline struct _hash_node* createNode(const char * const key,const size_t keysize,void *data,const size_t datasize)
{
    struct _hash_node *pnode = (struct _hash_node*)malloc(sizeof(struct _hash_node));
    pnode->key = (char*)malloc(keysize + 1);
    pnode->next = NULL;
    strncpy(pnode->key,key,keysize);
    pnode->key[keysize] = NULCHAR;
    pnode->data = malloc(datasize);
    memcpy(pnode->data,data,datasize);
    pnode->datasize = datasize;
    return pnode;
}

char* trimString(char *str)
{
    char *c = str;
    char *e;
    int len = strlen(str);
    while(*c != 0 && isspace(*c))
        ++c;
    if(*c != 0 && len > 0)
    {
        e = str + len - 1;
        while(e != c && isspace(*e))
            --e;
        *(++e) = 0;
    }
    return c;
}
void toLowerString(char *str)
{
    if(str)
    {
        while(0 != *str)
        {
            *str=tolower(*str);
            ++str;
        }
    }
}

struct _hash_table* createHashTab()
{
    struct _hash_table *ptab=(struct _hash_table*)malloc(sizeof(struct _hash_table));
    memset(ptab,0,sizeof(struct _hash_table));
    return ptab;
}

int hashTabPut(struct _hash_table *table,char *key,size_t keysize,void *data,size_t datasize)
{
    if(!table || !key || keysize == 0 || !data || datasize == 0)
        return 10;
    uint64_t h = hash(key);
    struct _hash_node *prev = table->bucks[h], *curr = table->bucks[h];
    int ret;
    while(NULL != curr && (ret = strcmp(curr->key, key)) < 0)
    {
        prev = curr;
        curr = curr->next;
    }
    if(NULL != curr && 0 == ret)//find a key conflict
    {
        free(curr->data);
        curr->data = malloc(datasize);
        memcpy(curr->data,data,datasize);
    }
    else
    {
        struct _hash_node *pnode = createNode(key, keysize, data, datasize);
        ++table->size;
        if(prev == curr)//either the key is less than the key in the first element or there is currently no node in the bucket yet
        {
            table->bucks[h] = pnode;
            pnode->next = curr;
        }
        else
        {
            prev->next = pnode;
            pnode->next = curr;
        }
    }
    return 0;
}
void* hashTabLookup(struct _hash_table *table,const char * const key,size_t keysize,size_t *pdatasize,HASH_LOOKUP_MOD mod)
{
    void *data=NULL;
    if(NULL != table && NULL != pdatasize && NULL != key)
    {
        uint64_t h = hash(key);
        struct _hash_node *node,*prev;
        int ret;
        prev=node=table->bucks[h];
        while(NULL != node&& (ret = strncmp(node->key,key,keysize)) < 0)
        {
            prev=node;
            node=node->next;
        }
        if(node!=NULL && 0 == ret)
        {
            data=malloc(node->datasize);
            memcpy(data,node->data,node->datasize);
            *pdatasize=node->datasize;
            if(mod==REMOVE)
            {
                if(prev == node)
                    table->bucks[h] = node->next;
                else
                    prev->next = node->next;
                free(node);
                table->size--;
            }
        }
    }
    return data;
}
size_t gethashTabSize(struct _hash_table *table)
{
    return table->size;
}
void hashTabDestroy(struct _hash_table *table)
{
    if(table)
    {
        struct _hash_node *pnext,*pcurrent;
        int i=0;
        for(;i<NBUCKETS;++i)
        {
            pcurrent=table->bucks[i];
            while(pcurrent)
            {
                pnext=pcurrent->next;
                free(pcurrent->key);
                free(pcurrent->data);
                free(pcurrent);
                pcurrent=pnext;
            }
        }
        free(table);
    }
}
struct _hash_table* getProperties(const char *path)
{
    struct _hash_table *hash = NULL;
    if(path)
    {
        FILE *pfile=fopen(path,"r");
        if(pfile)
        {
            flockfile(pfile);
            char buf[READBUFFSIZ];
            memset(buf,0,READBUFFSIZ);
            int i1,i2;
            char *k;
            hash = createHashTab();
            while(fgets(buf,READBUFFSIZ,pfile))
            {
                i1 = i2 = 0;
                while(i2 < READBUFFSIZ && *(buf + i2) != NEWLINE && *(buf + i2) != NULCHAR)
                {
                    if('=' == *(buf+i2))
                    {
                        i1 = i2;
                        *(buf + i2) = NULCHAR;
                    }
                    ++i2;
                }
                if(NEWLINE == *(buf + i2))
                    *(buf+i2) = NULCHAR;
                if(i1 > 0 && i2 < READBUFFSIZ - 2)
                {
                    /*trim key*/
                    k = trimString(buf);
                    toLowerString(k);
                    if(0 == *k)
                        printError(0,"Key can't be empty.");
                    else if(0 != hashTabPut(hash,k,strlen(k),buf + i1 + 1,i2 - i1))
                        printError(0,"Failed to hash %s.",buf);
                }
                else
                {
                    printError(0,"Key-value pair should not exceed %d characters.",READBUFFSIZ-1);
                    break;
                }
            }
            if(hash->size == 0)
            {
                hashTabDestroy(hash);
                hash=NULL;
            }
            funlockfile(pfile);
            fclose(pfile);
        }
    }
    return hash;
}

static int _iterateRegFileInDir(const char* dirName,DIR_SEARCH_MOD mode,void (*fun)(const char*))
{
    if(dirName && fun)
    {
        struct stat statbuf;
        struct stat statb;
        struct dirent *dirp = NULL;
        DIR *dp = NULL;
        int ret = 0;
        if((ret = lstat(dirName,&statbuf)) == -1)
            printError(ret,"Failed to lstat %s",dirName);
        else
        {
            if(0 == S_ISDIR(statbuf.st_mode))
                printError(0,"%s is not a directory",dirName);
            else
            {
                if((dp = opendir(dirName)) == NULL)
                    printError(1,"Can not open %s",dirName);
                else
                {
                    char *ptr=(char*)(dirName + strlen(dirName));
                    *ptr = DIRECTORY_SEPARATOR;
                    *++ptr = 0;
                    while((dirp = readdir(dp))!=NULL)
                    {
                        if(0 == strcmp(dirp->d_name,".") || 0 == strcmp(dirp->d_name,".."))
                            continue;
                        strcpy(ptr,dirp->d_name);
                        if((ret = lstat(dirName,&statb)) == -1)
                            printError(ret,"Failed to lstat %s",dirName);
                        else
                        {
                            if(S_IFREG == (statb.st_mode & S_IFMT))
                                fun(dirName);
                            else if(S_IFDIR == (statb.st_mode&S_IFMT)&& RESURSIVE == mode)
                            {
                                if((ret = _iterateRegFileInDir(dirName,mode,fun)) != 0)
                                    break;
                            }
                        }
                    }
                    ptr[-1] = 0;
                    if((ret = closedir(dp)) == -1)
                        printError(ret,"Failed to close directory %s",dirName);
                    return ret;
                }
            }
        }
    }
    return 1;
}
int iterateRegFileInDir(const char* dirName,DIR_SEARCH_MOD mode,void (*fun)(const char*))
{
    int irfi_len = 0;
    char *irfi_path = unix_path_alloc(&irfi_len);
    strncpy(irfi_path,dirName,irfi_len);
    int ret = _iterateRegFileInDir(dirName,mode,fun);
    if(irfi_len>0)
        free(irfi_path);
    irfi_len = 0;
    return ret;
}
char* unix_path_alloc(int *sizep)
{
    char *ptr;
    int size;
    if(posix_version == 0)
        posix_version = sysconf(_SC_VERSION);
    if(pathmax == 0)/*first time through*/
    {
        errno=0;
        if((pathmax = pathconf("/",_PC_PATH_MAX)) < 0)
        {
            if(0 == errno)
                pathmax = PATH_MAX_GUESS;
            else
                printError(errno,"pathconf error for _PC_PATH_MAX");
        }
        else
            pathmax++;/*add one since it's relative to root*/
    }
    if(posix_version < SUSV3)
        size = pathmax+1;
    else
        size = pathmax;
    if((ptr=malloc(size))==NULL)
        printError(errno,"malloc error for pathname");
    if(sizep!=NULL)
        *sizep=size;
    return ptr;
}

int createGrid(const char *pname,const char *playername,int epsg,GridCell *pcells,int count)
{
    if(!pname||!playername||!pcells)return 1;
    OGRDataSourceH hDs=NULL;
    OGRSpatialReferenceH hSRS=NULL;
    OGRSFDriverH hDriver=NULL;
    OGRLayerH hLayer=NULL;
    OGRFeatureDefnH hFtrDef=NULL;
    OGRFeatureH hFeature=NULL;
    OGRGeometryH hGeom=NULL;
    OGRGeometryH hRing=NULL;
    OGRRegisterAll();
    if(NULL == (hSRS = OSRNewSpatialReference(NULL)))
    {
        printError(1,"OGR can not create spatial reference");
        return 11;
    }
    OSRImportFromEPSG(hSRS,epsg);
    if((hDriver = OGRGetDriverByName("ESRI Shapefile")) == NULL)
    {
        printError(1,"OGR can not create driver for %s\n",pname);
        return 12;
    }
    if((hDs = OGR_Dr_CreateDataSource(hDriver,pname,NULL)) == NULL)
    {
        printError(1,"OGR can not create data source for %s, file may already exist!\n",pname);
        return 13;
    }
    if((hLayer = OGR_DS_CreateLayer(hDs,playername,hSRS,wkbPolygon,NULL)) == NULL)
    {
        printError(1,"OGR can not create polygon layer for %s\n",pname);
        return 14;
    }
    if((hFtrDef = OGR_L_GetLayerDefn(hLayer)) == NULL)
    {
        printError(1,"OGR can not retrieve layer defination for %s\n",pname);
        return 15;
    }
    int i=0;
    for(;i < count;++i)
    {
        hFeature=OGR_F_Create(hFtrDef);
        hGeom=OGR_G_CreateGeometry(wkbPolygon);
        hRing=OGR_G_CreateGeometry(wkbLinearRing);
        if((pcells + i)->length <= 4)
        {
            OGR_G_AddPoint_2D(hRing,(pcells+i)->coord1.x,(pcells+i)->coord1.y);
            OGR_G_AddPoint_2D(hRing,(pcells+i)->coord2.x,(pcells+i)->coord2.y);
            OGR_G_AddPoint_2D(hRing,(pcells+i)->coord3.x,(pcells+i)->coord3.y);
            if(4 == (pcells+i)->length)
                OGR_G_AddPoint_2D(hRing,(pcells+i)->coord4.x,(pcells+i)->coord4.y);
            OGR_G_AddPoint_2D(hRing,(pcells+i)->coord1.x,(pcells+i)->coord1.y);
        }
        else
        {
            Coordinate *ptr=(pcells+i)->pcoords;
            if(ptr)
            {
                int j = (pcells + i)->length;
                while(j--)
                {
                    OGR_G_AddPoint_2D(hRing,ptr->x,ptr->y);
                    ++ptr;
                }
                OGR_G_AddPoint_2D(hRing,(pcells + i)->pcoords->x,(pcells + i)->pcoords->y);
            }
            else
            {
                printError(1,"Coordinates are not filled for %s\n",pname);
                return 16;
            }
        }
        OGR_G_AddGeometry(hGeom,hRing);
        OGR_F_SetGeometry(hFeature,hGeom);
        OGR_L_CreateFeature(hLayer,hFeature);
        OGR_G_DestroyGeometry(hRing);
        OGR_G_DestroyGeometry(hGeom);
        OGR_F_Destroy(hFeature);
    }
    OSRDestroySpatialReference( hSRS );
    OGR_DS_Destroy(hDs);
    return 0;
}
