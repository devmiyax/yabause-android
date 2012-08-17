package org.yabause.android;

import java.util.ArrayList;
import java.util.List;
import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ListView;
import android.widget.TextView;

// TODO: Comment thoroughly
public class MenuSettingsInputConfigureButtonsActivity extends ListActivity implements IScancodeListener
{
    public static MenuSettingsInputConfigureButtonsActivity mInstance = null;
    private OptionArrayAdapter optionArrayAdapter;  // array of menu options
    private ScancodeDialog scancodeDialog = null;
    public static int controllerNum = -1;
    public static boolean plugged = true;

    @Override
    public void onCreate( Bundle savedInstanceState )
    {
        super.onCreate( savedInstanceState );
        mInstance = this;

        List<MenuOption>optionList = new ArrayList<MenuOption>();

        // TODO: remove (function in the background)
        optionList.add( new MenuOption( "Disable Volume Keys", "use as controller buttons", "menuSettingsInputConfigureVolume",
                                        Globals.volumeKeysDisabled ) );

        if( controllerNum > 0 && controllerNum < 3 )
        {
            if( controllerNum == 1 )
                plugged = true;
            else
                plugged = false;

            String val = MenuActivity.gui_cfg.get( "Controller" + controllerNum, "plugged" );
            if( val != null )
                plugged = ( val.equals( "True" ) ? true : false );

            optionList.add( new MenuOption( "Plugged In", "Connect Controller " + controllerNum, "plugged", plugged ) );
        }

        addOption( optionList, "D-pad Right", "DPad R" );
        addOption( optionList, "D-pad Left", "DPad L" );
        addOption( optionList, "D-pad Down", "DPad D" );
        addOption( optionList, "D-pad Up", "DPad U" );
        addOption( optionList, "Start", "Start" );
        addOption( optionList, "A", "A Button" );
        addOption( optionList, "B", "B Button" );
        addOption( optionList, "C", "C Button" );
        addOption( optionList, "X", "X Button" );
        addOption( optionList, "Y", "Y Button" );
        addOption( optionList, "Z", "Z Button" );
        addOption( optionList, "R", "R Trig" );
        addOption( optionList, "L", "L Trig" );
        optionList.add( new MenuOption( "--todo: NiGHTS ctrl analog--", "", "line" ) );
//        addOption( optionList, "Analog Right", "X Axis2" );
//        addOption( optionList, "Analog Left", "X Axis1" );
//        addOption( optionList, "Analog Down", "Y Axis2" );
//        addOption( optionList, "Analog Up", "Y Axis1" );

        optionArrayAdapter = new OptionArrayAdapter( this, R.layout.menu_option, optionList );
        setListAdapter( optionArrayAdapter );
    }

    public void addOption( List<MenuOption> optionList, String name, String info )
    {
        if( controllerNum < 1 || controllerNum > 2 || info == null )
            return;

        int scancode = 0;
        String val;

        if( info.contains( "Axis" ) )
            val = MenuActivity.gui_cfg.get( "Controller" + controllerNum,
                                                    info.substring( 0, info.length() - 1 ) );
        else
            val = MenuActivity.gui_cfg.get( "Controller" + controllerNum, info.substring( 0, info.length() ) );

        if( val == null )
            return;

        int x = val.indexOf( "(" );
        int y = val.indexOf( ")" );
        if( x < 0 || y < 0 || y <= x )
            return;
        val = val.substring( x + 1, y ).trim();

        if( val == null || val.length() < 1 )
            return;

        if( info.contains( "Axis" ) )
        {
            x = val.indexOf( "," );
            if( x < 0 )
                return;
            try
            {  // make sure a valid integer was entered
                if( info.contains( "Axis1" ) )
                {
                    scancode = Integer.valueOf( val.substring( 0, x ) ).intValue();
                }
                else
                {
                    scancode = Integer.valueOf( val.substring( x + 1, val.length() ) ).intValue();
                }
            }
            catch( NumberFormatException nfe )
            {}  // skip it if this happens
        }
        else
        {
            try
            {  // make sure a valid integer was entered
                scancode = Integer.valueOf( val ).intValue();
            }
            catch( NumberFormatException nfe )
            {}  // skip it if this happens
        }

        optionList.add( new MenuOption( name,
                                        ((scancode > 0) ? ("keycode " + scancode) : "(not mapped)"),
                                        info ) );
    }

    public void returnCode( int scancode, int codeType )
    {
        String param = ScancodeDialog.menuItemInfo;
        if( param == null )
            return;
        param = param.trim();
        String val;

        if( param.contains( "Axis" ) )
            val = MenuActivity.gui_cfg.get( "Controller" + controllerNum,
                                                    param.substring( 0, param.length() - 1 ) );
        else
            val = MenuActivity.gui_cfg.get( "Controller" + controllerNum,
                                                    param.substring( 0, param.length() ) );

        if( val == null )
            return;

        int x = val.indexOf( "(" );
        int y = val.indexOf( ")" );
        if( x < 0 || y < 0 || y <= x )
            return;

        val = val.substring( x + 1, y ).trim();

        if( param.contains( "Axis" ) )
        {
            x = val.indexOf( "," );
            if( x < 0 )
                return;
            if( param.contains( "Axis1" ) )
                val = "(" + scancode + "," + val.substring( x + 1, val.length() ) + ")";
            else
                val = "(" + val.substring( 0, x ) + "," + scancode + ")";
            MenuActivity.gui_cfg.put( "Controller" + controllerNum,
                                              param.substring( 0, param.length() - 1 ), "key" + val );
        }
        else
        {
            val = "(" + scancode + ")";
            MenuActivity.gui_cfg.put( "Controller" + controllerNum, param, "key" + val );
        }
        optionArrayAdapter.remove( optionArrayAdapter.getOption( ScancodeDialog.menuItemPosition ) );
        optionArrayAdapter.insert( new MenuOption( ScancodeDialog.menuItemName,
                                                   ((scancode > 0) ? ("keycode " + scancode) : "(not mapped)"),
                                                   param ),
                                   ScancodeDialog.menuItemPosition );
    }

    /*
     * Determines what to do, based on what option the user chose 
     * @param listView Used by Android.
     * @param view Used by Android.
     * @param position Which item the user chose.
     * @param id Used by Android.
     */
    @Override
    protected void onListItemClick( ListView listView, View view, int position, long id )
    {
        super.onListItemClick( listView, view, position, id );
        MenuOption menuOption = optionArrayAdapter.getOption( position );
        if( scancodeDialog == null )
            scancodeDialog = new ScancodeDialog( mInstance );

        if( menuOption.info.equals( "menuSettingsInputConfigureVolume" ) ) 
        {
            Globals.volumeKeysDisabled = !Globals.volumeKeysDisabled;
            MenuActivity.gui_cfg.put( "KEYS", "disable_volume_keys", Globals.volumeKeysDisabled ? "1" : "0" );
            optionArrayAdapter.remove( menuOption );
            optionArrayAdapter.insert( new MenuOption( "Disable Volume Keys", "use as controller buttons",
                                           "menuSettingsInputConfigureVolume", Globals.volumeKeysDisabled ), position );
        }
        else if( menuOption.info.equals( "plugged" ) ) 
        {
            plugged = !plugged;

            MenuActivity.gui_cfg.put( "Controller" + controllerNum, "plugged", plugged ? "True" : "False" );
            optionArrayAdapter.remove( menuOption );
            optionArrayAdapter.insert( new MenuOption( "Plugged In", "Enable to connect Controller " + controllerNum,
                                                       "plugged", plugged ), position );
        }
        else if( !menuOption.info.equals( "line" ) )
        {
            ScancodeDialog.parent = this;
            ScancodeDialog.codeType = 0;

            ScancodeDialog.menuItemName = menuOption.name;
            ScancodeDialog.menuItemInfo = menuOption.info;
            ScancodeDialog.menuItemPosition = position;
            scancodeDialog.show();
        }
    }
}
