
extern APTR old_ppc_func_GetBitMapAttr;
extern APTR old_ppc_func_LockBitMapTagList;
extern APTR old_ppc_func_UnlockBitMap;

extern ULONG fake_GetBitMapAttr ( struct BitMap * bitMap, ULONG value );
extern APTR  fake_LockBitMapTagList( struct BitMap * bitMap, struct TagItem * tags );
extern void  fake_UnlockBitMap(APTR lock);

extern ULONG ppc_func_GetBitMapAttr (struct GraphicsIFace *Self, struct BitMap *bitMap, ULONG value);

extern APTR  ppc_func_LockBitMapTagList(struct GraphicsIFace *Self, struct BitMap * bitMap, struct TagItem * tags);
extern void  ppc_func_UnlockBitMap(struct GraphicsIFace *Self, APTR lock);

