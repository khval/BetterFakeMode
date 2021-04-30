
#define set_new_68k_patch( lib, name ) \
	old_68k_stub_ ## name = IExec -> SetFunction( lib ## Base, _LVO ## name, (APTR) &(stub_68k_ ## name)); \
	if ( ! old_68k_stub_ ## name) return FALSE;

#define undo_68k_patch( lib, name ) \
	if (old_68k_stub_ ## name) IExec -> SetFunction( lib ## Base, _LVO ## name, old_68k_stub_ ## name); \
	old_68k_stub_ ## name = NULL;

// offsetof( (struct IntuitionIFace),  name )

#define set_new_ppc_patch(lib,name) \
	old_ppc_func_ ## name = IExec->SetMethod( (struct Interface *) I ##lib, offsetof(struct lib ## IFace,name ) , (APTR) ppc_func_ ## name); \
	if ( ! old_ppc_func_## name) return FALSE;

#define undo_ppc_patch(lib,name) \
	if (old_ppc_func_ ## name) IExec->SetMethod( (struct Interface *) I ## lib, offsetof(struct lib ## IFace,name ) , (APTR) old_ppc_func_ ## name); \
	old_ppc_func_ ## name = NULL;

