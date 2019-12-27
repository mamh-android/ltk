<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:java="http://xml.apache.org/xslt/java"
                exclude-result-prefixes="java">

  <xsl:output method="xml" doctype-system="stax.dtd"/>

  <xsl:template match="@* | node() | text() | comment() | processing-instruction()">
    <xsl:copy>
      <xsl:call-template name="processEachNodeAndAttribute"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template name="processEachNodeAndAttribute" match="@* | node()">
    <xsl:copy>
      <xsl:attribute name="_ln">
        <xsl:value-of select="java:org.apache.xalan.lib.NodeInfo.lineNumber()"/>
      </xsl:attribute>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>