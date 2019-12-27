<?xml version="1.0" encoding="iso-8859-1"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="html"/>

  <xsl:template match="stax">
    <html>
      <head>
        <title>
          <xsl:text>STAX Function Definitions</xsl:text>
        </title>
      </head>
      <body>
        <H1><xsl:text>STAX Function Definitions</xsl:text></H1>
        <xsl:apply-templates/>
      </body>
    </html>
  </xsl:template>

  <!-- Ignore these elements contained by the stax element -->
  <xsl:template match="script"/>
  <xsl:template match="defaultcall"/>
  <xsl:template match="signalhandler"/>

  <xsl:template match="function">
    <hr WIDTH="100%"/>
    <h2><u><xsl:value-of select="@name"/></u></h2>
    <xsl:apply-templates select="function-description|function-prolog|function-no-args|function-single-arg|function-list-args|function-map-args|function-other-args"/>
    <xsl:apply-templates select="function-epilog"/>
  </xsl:template>

  <xsl:template match="function-description">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="function-description/text()">
    <p><xsl:value-of disable-output-escaping="yes" select="."/></p>
  </xsl:template>

  <xsl:template match="function-description/comment()">
    <pre><xsl:value-of select="."/></pre>
  </xsl:template>

  <xsl:template match="function-prolog">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="function-prolog/text()">
    <p><xsl:value-of disable-output-escaping="yes" select="."/></p>
  </xsl:template>

  <xsl:template match="function-prolog/comment()">
    <pre><xsl:value-of select="."/></pre>
  </xsl:template>

  <xsl:template match="function-no-args">
    <p>
    <table BORDER="1">
      <tbody>
        <tr>
          <td COLSPAN="2"><b>This function does not allow any arguments</b></td>
        </tr>
        <xsl:apply-templates mode="single-arg"/>
      </tbody>
    </table>
    </p>
  </xsl:template>

  <xsl:template match="function-single-arg">
    <p>
    <table BORDER="1">
      <thead>
        <tr>
          <th COLSPAN="6"><b>This function takes a single argument</b></th>
        </tr>
        <tr>
          <th><b>Name</b></th>
          <th><b>Description</b></th>
          <th><b>Required</b></th>
          <th><b>Private</b></th>
          <th><b>Default</b></th>
          <th><b>Properties</b></th>
        </tr>
      </thead>
      <tbody>
        <xsl:apply-templates/>
      </tbody>
    </table>
    </p>
  </xsl:template>

  <xsl:template match="function-list-args">
    <p>
    <table BORDER="1">
      <thead>
        <tr>
          <th COLSPAN="6"><b>This function takes a list of arguments</b></th>
        </tr>
        <tr>
          <th><b>Name</b></th>
          <th><b>Description</b></th>
          <th><b>Required</b></th>
          <th><b>Private</b></th>
          <th><b>Default</b></th>
          <th><b>Properties</b></th>
        </tr>
      </thead>
      <tbody>
        <xsl:apply-templates/>
      </tbody>
    </table>
    </p>
  </xsl:template>

  <xsl:template match="function-map-args">
    <p>
    <table BORDER="1">
      <thead>
        <tr>
          <th COLSPAN="6"><b>This function takes an argument map</b></th>
        </tr>
        <tr>
          <th><b>Name</b></th>
          <th><b>Description</b></th>
          <th><b>Required</b></th>
          <th><b>Private</b></th>
          <th><b>Default</b></th>
          <th><b>Properties</b></th>
        </tr>
      </thead>
      <tbody>
        <xsl:apply-templates/>
      </tbody>
    </table>
    </p>
  </xsl:template>

  <xsl:template match="function-required-arg">
    <tr>
      <td><xsl:value-of select="@name"/></td>
      <td><xsl:apply-templates/></td>
      <td>Yes</td>
      <td>No</td>
      <td>N/A</td>
      <td></td>
    </tr>
  </xsl:template>

  <xsl:template match="function-required-arg/text()">
    <p><xsl:value-of disable-output-escaping="yes" select="."/></p>
  </xsl:template>

  <xsl:template match="function-required-arg" mode="single-arg">
    <tr>
    <td><b>Name</b></td>
    <td><xsl:value-of select="@name"/></td>
    </tr>
    <tr>
    <td><b>Description</b></td>
    <td><xsl:apply-templates/></td>
    </tr>
    <tr>
    <td><b>Required</b></td>
    <td>Yes</td>
    </tr>
    <tr>
    <td><b>Private</b></td>
    <td>No</td>
    </tr>
    <tr>
    <td><b>Default</b></td>
    <td>N/A</td>
    </tr>
    <tr>
    <td><b>Properties</b></td>
    <td></td>
    </tr>
  </xsl:template>

  <xsl:template match="function-optional-arg">
     <tr>
      <td><xsl:value-of select="@name"/></td>
      <td><xsl:apply-templates/></td>
      <td>No</td>
      <td>No</td>
      <td><xsl:value-of select="@default"/></td>
      <td></td>
    </tr>
  </xsl:template>

  <xsl:template match="function-other-args">
     <tr>
      <td><xsl:value-of select="@name"/></td>
      <td COLSPAN="3"><xsl:apply-templates/></td>
    </tr>
  </xsl:template>

  <xsl:template match="function-optional-arg" mode="single-arg">
    <tr>
    <td><b>Name</b></td>
    <td><xsl:value-of select="@name"/></td>
    </tr>
    <tr>
    <td><b>Description</b></td>
    <td><xsl:apply-templates/></td>
    </tr>
    <tr>
    <td><b>Required</b></td>
    <td>No</td>
    </tr>
    <tr>
    <td><b>Private</b></td>
    <td>No</td>
    </tr>
    <tr>
    <td><b>Default</b></td>
    <td><xsl:value-of select="@default"/></td>
    </tr>
    <tr>
    <td><b>Properties</b></td>
    <td></td>
    </tr>
  </xsl:template>

  <xsl:template match="function-arg-def">
    <tr>
      <td><xsl:value-of select="@name"/></td>
      <td><xsl:apply-templates select="./function-arg-description"/></td>
      <xsl:choose>
        <xsl:when test="@type = 'required'">
          <td>Yes</td>
        </xsl:when>
        <xsl:when test="@type = 'optional'">
          <td>No</td>
        </xsl:when>
        <xsl:when test="@type = 'other'">
          <td>Other</td>
        </xsl:when>
      </xsl:choose>
      <xsl:choose>
        <xsl:when test="count(./function-arg-private) > 0">
          <td>Yes</td>
        </xsl:when>
        <xsl:otherwise>
          <td>No</td>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
        <xsl:when test="@type = 'required'">
          <td>N/A</td>
        </xsl:when>
        <xsl:otherwise>
          <td><xsl:value-of select="@default"/></td>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:if test="count(./function-arg-property) > 0">
        <td>
          <table BORDER="1">
            <thead>
              <tr>
                <th><b>Name</b></th>
                <th><b>Value</b></th>
                <xsl:if test="count(./function-arg-property/function-arg-property-description) > 0">
                  <th><b>Description</b></th>
                </xsl:if>
                <xsl:if test="count(./function-arg-property/function-arg-property-data) > 0">
                  <th><b>Data</b></th>
                </xsl:if>
              </tr>
            </thead>
            <xsl:for-each select="./function-arg-property">
              <tr>
                <td><xsl:apply-templates select="@name"/></td>
                <td><xsl:apply-templates select="@value"/></td>
                <xsl:if test="count(function-arg-property-description) > 0">
                  <td><xsl:apply-templates select="function-arg-property-description"/></td>
                </xsl:if>
                <xsl:if test="count(function-arg-property-data) > 0">
                  <td>
                    <table BORDER="1">
                      <thead>
                        <tr>
                          <th><b>Type</b></th>
                          <th><b>Value</b></th>
                          <xsl:if test="count(function-arg-property-data/function-arg-property-data) > 0">
                           <th><b>Data</b></th>
                          </xsl:if>
                        </tr>
                      </thead>
                      <xsl:for-each select="function-arg-property-data">
                        <tr>
                          <td><xsl:apply-templates select="@type"/></td>
                          <td><xsl:apply-templates select="@value"/></td>
                          <xsl:if test="count(function-arg-property-data) > 0">
                            <td>
                              <table BORDER="1">
                                <thead>
                                  <tr>
                                    <th><b>Type</b></th>
                                    <th><b>Value</b></th>
                                  </tr>
                                </thead>
                                <xsl:for-each select="function-arg-property-data">
                                  <tr>
                                    <td><xsl:apply-templates select="@type"/></td>
                                    <td><xsl:apply-templates select="@value"/></td>
                                  </tr>
                                </xsl:for-each>
                              </table>
                            </td>
                          </xsl:if>
                        </tr>
                      </xsl:for-each>
                    </table>
                  </td>
                </xsl:if>
              </tr>
            </xsl:for-each>
          </table>
        </td>
      </xsl:if>
    </tr>
  </xsl:template>

  <xsl:template match="function-epilog">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="function-epilog/text()">
    <p><xsl:value-of disable-output-escaping="yes" select="."/></p>
  </xsl:template>

  <xsl:template match="function-epilog/comment()">
    <pre><xsl:value-of select="."/></pre>
  </xsl:template>

</xsl:stylesheet>
