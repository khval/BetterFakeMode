
#define set_new_68k_patch( name ) \
	old_68k_stub_ ## name = IExec -> SetFunction( IntuitionBase, _LVO ## name, (APTR) &(stub_68k_ ## name)); \
	if ( ! old_68k_stub_ ## name) return FALSE;

#define undo_68k_patch( name ) \
	if (old_68k_stub_ ## name) IExec -> SetFunction( IntuitionBase, _LVO ## name, old_68k_stub_ ## name); \
	old_68k_stub_ ## name = NULL;

// offsetof( (struct IntuitionIFace),  name )

#define set_new_ppc_patch( name ) \
	old_ppc_func_ ## name = IExec->SetMethod( (struct Interface *) IIntuition, offsetof(struct IntuitionIFace,name ) , (APTR) ppc_func_ ## name); \
	if ( ! old_ppc_func_## name) return FALSE;

#define undo_ppc_patch( name ) \
	if (old_ppc_func_ ## name) IExec->SetMethod( (struct Interface *) IIntuition, offsetof(struct IntuitionIFace,name ) , (APTR) old_ppc_func_ ## name); \
	old_ppc_func_ ## name = NULL;

