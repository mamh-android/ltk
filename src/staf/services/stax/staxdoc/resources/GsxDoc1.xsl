<?xml version="1.0" encoding="iso-8859-1"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="html" />

	<xsl:template match="stax">

		<TABLE BORDER="1" CELLPADDING="3" CELLSPACING="0" WIDTH="100%">
			<TR BGCOLOR="#CCCCFF">
				<TD COLSPAN="2">
					<FONT SIZE="+2">
						<B>Function Summary</B>
					</FONT>
				</TD>
			</TR>

			<xsl:apply-templates />

		</TABLE>
		<BR></BR>
		<BR></BR>

	</xsl:template>

   <!-- Ignore these elements contained by the stax element -->
	<xsl:template match="script"/>
    <xsl:template match="defaultcall"/>
    <xsl:template match="signalhandler"/>

	<xsl:template match="function">
		<TR>
			<TD style="vertical-align: top;">
				<CODE>
					<B>
						<A HREF="#{@name}">
							<xsl:value-of select="@name" />
						</A>
					</B>
				</CODE>
			</TD>
			<TD>
				<xsl:apply-templates select="function-description|function-prolog" />
			</TD>
		</TR>

	</xsl:template>

	<xsl:template match="function-description">
		<xsl:apply-templates />
	</xsl:template>

    <!-- Show only the first sentence (ending with a period, '.') of the function-description
          in the function summary. -->
	<xsl:template match="function-description">
        <xsl:variable name="txt" select="concat((.//text())[1], (.//text())[2])"/>
        <xsl:if test="substring-before($txt, '.') != ''">
            <xsl:value-of disable-output-escaping="yes" select="substring-before($txt, '.')" />.
        </xsl:if>
        <xsl:if test="substring-before($txt, '.') = ''">
            <xsl:value-of disable-output-escaping="yes" select="$txt"/>
        </xsl:if>
    </xsl:template>

    <xsl:template match="function-description/comment()">
		<pre>
			<xsl:value-of select="." />
		</pre>
    </xsl:template>

	<xsl:template match="function-prolog">
		<xsl:apply-templates />
	</xsl:template>

    <!-- Show only the first sentence (ending with a period, '.') of the function-prolog
          in the function summary. -->
	<xsl:template match="function-prolog">
        <xsl:variable name="txt" select="concat((.//text())[1], (.//text())[2])"/>
        <xsl:if test="substring-before($txt, '.') != ''">
            <xsl:value-of disable-output-escaping="yes" select="substring-before($txt, '.')" />.
        </xsl:if>
        <xsl:if test="substring-before($txt, '.') = ''">
            <xsl:value-of disable-output-escaping="yes" select="$txt"/>
        </xsl:if>
    </xsl:template>
    
    <xsl:template match="function-prolog/comment()">
		<pre>
			<xsl:value-of select="." />
		</pre>
	</xsl:template>
   


</xsl:stylesheet>
