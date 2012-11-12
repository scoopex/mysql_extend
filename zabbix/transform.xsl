<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
  <html>
  <head></head>
  <style type="text/css">

   p.desc{
     white-space:nowrap;
   }

   table.pretty {
    margin: 1em 1em 1em 2em;
    background: whitesmoke;
    border-collapse: collapse;
   }

   table.pretty th, table.pretty td {
    border: 1px gainsboro solid;
    padding: 0.2em;
   }

   table.pretty th {
    background: gainsboro;
    text-align: left;
   }

   table.pretty caption {
    margin-left: inherit;
    margin-right: inherit;
    white-space:nowrap;
   }
  </style>
  <body>
  <h2>Macro Overview</h2>

  The following macros can be overloaded on host level.
  <table class="pretty">
    <tr>
      <th>Name</th>
      <th>Default</th>
    </tr>
    <xsl:for-each select="zabbix_export/hosts/host/macros/macro">
    <tr>
      <td><xsl:value-of select="name"/></td>
      <td><xsl:value-of select="value"/></td>
    </tr>
    </xsl:for-each>
  </table>



  <h2>Trigger Overview</h2>
  <table class="pretty">
    <tr>
      <th>Description</th>
      <th>Priority</th>
      <th>Expression</th>
      <th>Comment</th>
    </tr>
    <xsl:for-each select="zabbix_export/hosts/host/triggers/trigger">
    <tr>
      <td><xsl:value-of select="description"/></td>
      <xsl:choose>
         <xsl:when test="priority='0'"><td style="background-color:#efefef;">Not classified (No alarm)</td></xsl:when>
         <xsl:when test="priority='1'"><td style="background-color:#FFFF00;">Information (Jabber)</td></xsl:when>
         <xsl:when test="priority='2'"><td style="background-color:#FFFF00;">Warning (Jabber)</td></xsl:when>
         <xsl:when test="priority='3'"><td style="background-color:#FF0000;">Average (SMS + Jabber)</td></xsl:when>
         <xsl:when test="priority='4'"><td style="background-color:#FF0000;">High (SMS +  Jabber)</td></xsl:when>
         <xsl:when test="priority='5'"><td style="background-color:#FF0000;">Disaster (SMS + Jabber></td></xsl:when>
         <xsl:otherwise><xsl:value-of select="priority"/><td>ERROR - Unknown</td></xsl:otherwise>
      </xsl:choose>
      <td><tt><xsl:value-of select="expression"/></tt></td>
      <td><tt><xsl:value-of select="comments"/></tt></td>
    </tr>
    </xsl:for-each>
  </table>

  <h2>Graph Overview</h2>
  <table class="pretty">
    <tr>
      <th>Name</th>
      <th>Element</th>
    </tr>
    <xsl:for-each select="zabbix_export/hosts/host/graphs/graph">
    <tr>
      <td><xsl:value-of select="@name"/></td>
      <td><tt><xsl:for-each select="graph_elements/graph_element">
         <xsl:value-of select="@item"/><br/>
      </xsl:for-each></tt></td>
    </tr>
    </xsl:for-each>
  </table>


  <h2>Item Overview</h2>
  <table class="pretty">
    <tr>
      <th>Type</th>
      <th>Key</th>
      <th>Description</th>
      <th>Interval (sec)</th>
      <th>History Days</th>
      <th>Trend Days</th>
    </tr>
    <xsl:for-each select="zabbix_export/hosts/host/items/item">
    <tr>
      <xsl:choose>
         <xsl:when test="@value_type='3'"><td><p class="desc">External check</p></td></xsl:when>
         <xsl:otherwise><xsl:value-of select="state"/><td>><p class="desc">ERROR - Unknown</p></td></xsl:otherwise>
      </xsl:choose>
      <td><tt><xsl:value-of select="@key"/></tt></td>
      <td><xsl:value-of select="description"/></td>
      <td><xsl:value-of select="delay"/></td>
      <td><xsl:value-of select="history"/></td>
      <td><xsl:value-of select="trends"/></td>
    </tr>
    </xsl:for-each>
  </table>
  </body>
  </html>
</xsl:template>
</xsl:stylesheet> 
