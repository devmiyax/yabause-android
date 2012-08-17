/*
Simple DirectMedia Layer
Java source code (C) 2009-2011 Sergii Pylypenko
  
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
  
1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required. 
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

// Portions of this code were taken from from Pelya's Android SDL port.
// THIS IS NOT THE ORIGINAL SOURCE, IT HAS BEEN ALTERED TO FIT THIS APP
// (05SEP2011, http://www.paulscode.com)

package org.yabause.android;

import android.view.KeyEvent;

class Globals
{
    public static String PackageName = "paulscode.android.yabause";
    public static String StorageDir = "/mnt/sdcard";
    public static String DataDir = StorageDir + "/Android/data/" + PackageName;
    public static String LibsDir = "/data/data/" + PackageName;
    public static boolean DataDirChecked = false;   // sdcard could be at "/sdcard"

    public static String DataDownloadUrl = "Data size is 1.0 Mb|yabause_data.zip";

    public static boolean DownloadToSdcard = true;

    public static String errorMessage = null;

    public static boolean InhibitSuspend = true;

    public static String chosenROM = null;
    public static String chosenBIOS = null;

    public static boolean volumeKeysDisabled = false;

    public static boolean redrawAll = true;
    public static boolean showFPS = true;
    public static boolean gamepadEnabled = true;
    public static String chosenGamepad = "Yabause-AE-Grey";

    // Controller configurations:
    public static boolean analog_100_64 = true; // IMEs where keycode * 100 + (0 --> 64)
    public static int[][] ctrlr = new int[2][17];
    // Button indexes:
    public static final int DPADU   =  0;
    public static final int DPADR   =  1;
    public static final int DPADD   =  2;
    public static final int DPADL   =  3;
    public static final int RTRIG   =  4;
    public static final int LTRIG   =  5;
    public static final int START   =  6;
    public static final int ABUTTON =  7;
    public static final int BBUTTON =  8;
    public static final int CBUTTON =  9;
    public static final int XBUTTON = 10;
    public static final int YBUTTON = 11;
    public static final int ZBUTTON = 12;
    public static final int ANALOGR = 13;
    public static final int ANALOGL = 14;
    public static final int ANALOGD = 15;
    public static final int ANALOGU = 16;
    public static final int BTN_SUM = 17;  // TOSO: add this for controller 2 button indexes

    // paulscode, added for the status bar icon:
    public static final int NOTIFICATION_ID = 10001;

    public static void populateControls()
    {
        String val = MenuActivity.gui_cfg.get( "KEYS", "disable_volume_keys" );
        if( val != null )
            volumeKeysDisabled = ( val.equals( "1" ) ? true : false );
        for( int p = 0; p < 2; p++ )
        {
            ctrlr[p][DPADR] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "DPad R" ), 0 );
            ctrlr[p][DPADL] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "DPad L" ), 0 );
            ctrlr[p][DPADD] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "DPad D" ), 0 );
            ctrlr[p][DPADU] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "DPad U" ), 0 );
            ctrlr[p][START] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "Start" ), 0 );
            ctrlr[p][ABUTTON] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "A Button" ), 0 );
            ctrlr[p][BBUTTON] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "B Button" ), 0 );
            ctrlr[p][CBUTTON] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "C Button" ), 0 );
            ctrlr[p][XBUTTON] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "X Button" ), 0 );
            ctrlr[p][YBUTTON] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "Y Button" ), 0 );
            ctrlr[p][ZBUTTON] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "Z Button" ), 0 );
            ctrlr[p][RTRIG] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "R Trig" ), 0 );
            ctrlr[p][LTRIG] = keyToInt( MenuActivity.gui_cfg.get( "Controller" + (p+1), "L Trig" ), 0 );
            val = MenuActivity.gui_cfg.get( "Controller" + (p+1), "X Axis" );
            if( val != null )
            {
                int x = val.indexOf( "(" );
                int y = val.indexOf( ")" );
                if( x >= 0 && y >= 0 && y > x )
                {
                    val = val.substring( x + 1, y ).trim();
                    x = val.indexOf( "," );
                    if( x >= 0 )
                    {
                        ctrlr[p][ANALOGR] = toInt( val.substring( x + 1, val.length() ), 0 );
                        ctrlr[p][ANALOGL] = toInt( val.substring( 0, x ), 0 );
                    }
                }
                val = MenuActivity.gui_cfg.get( "Controller" + (p+1), "Y Axis" );
                x = val.indexOf( "(" );
                y = val.indexOf( ")" );
                if( x >= 0 && y >= 0 && y > x )
                {
                    val = val.substring( x + 1, y ).trim();
                    x = val.indexOf( "," );
                    if( x >= 0 )
                    {
                        ctrlr[p][ANALOGD] = toInt( val.substring( x + 1, val.length() ), 0 );
                        ctrlr[p][ANALOGU] = toInt( val.substring( 0, x ), 0 );
                    }
                }
            }
        }
    }

    private static int keyToInt( String val, int fail )
    {
        int x = val.indexOf( "(" );
        int y = val.indexOf( ")" );
        if( x >= 0 && y >= 0 && y > x )
            return toInt( val.substring( x + 1, y ).trim(), fail );
        return fail;
    }
    /*
     * Converts a string into an integer.
     * @param val String containing the number to convert.
     * @param fail Value to use if unable to convert val to an integer.
     * @return The converted integer, or the specified value if unsucessful.
     */
    public static int toInt( String val, int fail )
    {
        if( val == null || val.length() < 1 )
            return fail;  // not a number
        try
        {
            return Integer.valueOf( val ).intValue();  // convert to integer
        }
        catch( NumberFormatException nfe )
        {}

        return fail;  // conversion failed
    }
    /*
     * Converts a string into a float.
     * @param val String containing the number to convert.
     * @param fail Value to use if unable to convert val to an float.
     * @return The converted float, or the specified value if unsucessful.
     */
    public static float toFloat( String val, float fail )
    {
        if( val == null || val.length() < 1 )
            return fail;  // not a number
        try
        {
            return Float.valueOf( val ).floatValue();  // convert to float
        }
        catch( NumberFormatException nfe )
        {}

        return fail;  // conversion failed
    }
}
