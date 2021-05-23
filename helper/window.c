
ULONG get_default_icon_size(struct RastPort *rp)
{
	ULONG icon_s = rp -> Font -> tf_YSize + 4;
	if (icon_s < 5) icon_s = 5;
	return icon_s;
}

