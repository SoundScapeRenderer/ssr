<?xml version="1.0"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!--
       NOTE: In Firefox, if the stylesheet doesn't work as a symbolic link:
       Type "about:config", search "security.fileuri.strict_origin_policy" and
       change it to "false".
   -->

  <xsl:template match="/">
    <html>
      <body>
        <h1>Audio Scene Description</h1>

        <xsl:for-each select="asdf/header">
          <h2>Header</h2>
          <xsl:for-each select="name">
            <p>
              Name:
              <em><xsl:value-of select="."/></em>
            </p>
          </xsl:for-each>
          <xsl:for-each select="description">
            <p>
              Description:
              <em><xsl:value-of select="."/></em>
            </p>
          </xsl:for-each>
          <xsl:for-each select="version">
            <p>
              Version:
              <em><xsl:value-of select="."/></em>
            </p>
          </xsl:for-each>
          <xsl:for-each select="date">
            <p>
              Date:
              <em><xsl:value-of select="."/></em>
            </p>
          </xsl:for-each>
        </xsl:for-each>

        <xsl:for-each select="asdf/reproduction_setup">
          <h2>Reproduction Setup</h2>

          <xsl:for-each select="loudspeaker">
            <h3>Single Loudspeaker</h3>
            <xsl:apply-templates select="@*"/>
            <xsl:apply-templates/>
          </xsl:for-each>

          <xsl:for-each select="circular_array">
            <h3>Circular Loudspeaker Array with <xsl:value-of select="@number"/>
              Loudspeakers</h3>
            <xsl:apply-templates select="@*"/>
            <xsl:apply-templates/>
          </xsl:for-each>

          <xsl:for-each select="linear_array">
            <h3>Linear Loudspeaker Array with <xsl:value-of select="@number"/>
              Loudspeakers</h3>
            <xsl:apply-templates select="@*"/>
            <xsl:apply-templates/>
          </xsl:for-each>

          <!-- <xsl:sort select="file"/> -->
          <!--
          <xsl:if test="position() < last()-1">
          </xsl:if>
          -->
        </xsl:for-each>

        <xsl:for-each select="asdf/scene_setup">
          <h2>Scene Setup</h2>

          <h3>Reference point</h3>
          <xsl:choose>
            <xsl:when test="reference">
              <xsl:for-each select="reference">
                <xsl:apply-templates select="@*"/>
                <xsl:apply-templates/>
              </xsl:for-each>
            </xsl:when>
            <xsl:otherwise>
              No reference specified.
            </xsl:otherwise>
          </xsl:choose>

          <xsl:for-each select="source">
            <h3>Source</h3>
            <xsl:apply-templates select="@*"/>
            <xsl:apply-templates/>
          </xsl:for-each>

        </xsl:for-each>

        <xsl:for-each select="asdf/score">
          <h2>Score</h2>
          <p>The score section is still in a very, very unfinished state!</p>

          <xsl:for-each select="event">
            <h3>Event</h3>
            <xsl:apply-templates/>
          </xsl:for-each>

        </xsl:for-each>

      </body>
    </html>
  </xsl:template>

  <xsl:template match="@name">
    <p>
      Name:
      <em><xsl:value-of select="."/></em>
    </p>
  </xsl:template>

  <xsl:template match="@id">
    <p>
      ID:
      <em><xsl:value-of select="."/></em>
    </p>
  </xsl:template>

  <xsl:template match="@number">
    <p>
      Number:
      <em><xsl:value-of select="."/></em>
    </p>
  </xsl:template>

  <xsl:template match="@model">
    <p>
      Model:
      <em><xsl:value-of select="."/></em>
    </p>
  </xsl:template>

  <xsl:template match="@delay">
    <p>
      Delay:
      <em><xsl:value-of select="."/></em>
    </p>
  </xsl:template>

  <xsl:template match="@weight">
    <p>
      Weight:
      <em><xsl:value-of select="."/></em>
    </p>
  </xsl:template>

  <xsl:template match="file">
    <p>
      File:
      <xsl:value-of select="."/>
      <xsl:if test="@channel">
        (channel # <xsl:value-of select="@channel"/>)
      </xsl:if>
    </p>
  </xsl:template>

  <xsl:template match="first">
    <h4>First Loudspeaker</h4>
    <xsl:apply-templates select="@*"/>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="second">
    <h4>Second Loudspeaker</h4>
    <xsl:apply-templates select="@*"/>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="last">
    <h4>Last Loudspeaker</h4>
    <xsl:apply-templates select="@*"/>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="position">
    <p>
      Position:
      (<xsl:value-of select="@x"/>,
      <xsl:value-of select="@y"/>
      <xsl:if test="@z">,
        <xsl:value-of select="@z"/>
      </xsl:if>)
    </p>
  </xsl:template>

  <xsl:template match="orientation">
    <p>
      Orientation:
      <xsl:value-of select="@azimuth"/> (azimuth)
      <xsl:if test="@elevation">,
        <xsl:value-of select="@elevation"/> (elevation)
      </xsl:if>
      <xsl:if test="@tilt">,
        <xsl:value-of select="@tilt"/> (tilt)
      </xsl:if>
    </p>
  </xsl:template>

  <xsl:template match="angle">
    <p>
      Angle:
      <xsl:value-of select="@azimuth"/> (azimuth)
      <xsl:if test="@elevation">,
        <xsl:value-of select="@elevation"/> (elevation)
      </xsl:if>
      <xsl:if test="@tilt">,
        <xsl:value-of select="@tilt"/> (tilt)
      </xsl:if>
    </p>
  </xsl:template>

</xsl:stylesheet>
