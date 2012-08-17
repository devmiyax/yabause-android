package org.yabause.android;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import android.app.ListActivity;
import android.app.NotificationManager;
import android.content.Intent;
import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

// TODO: Comment thoroughly
public class MenuActivity extends ListActivity implements IFileChooser
{
    public static MenuActivity mInstance = null;
    private OptionArrayAdapter optionArrayAdapter;  // array of menu options
    private static NotificationManager notificationManager = null;

    public static Config gui_cfg = null;
    public static Config error_log = null;
    public static String biosFile = "(not selected)";

    @Override
    public void onCreate( Bundle savedInstanceState )
    {
        super.onCreate( savedInstanceState );
        mInstance = this;

        if( notificationManager == null )
            notificationManager = (NotificationManager) getSystemService( Context.NOTIFICATION_SERVICE );
        notificationManager.cancel( Globals.NOTIFICATION_ID );

        if( gui_cfg == null || error_log == null || Globals.DataDir == null || Globals.DataDir.length() == 0 || !Globals.DataDirChecked )
        {
            Globals.PackageName = getPackageName();
            Globals.LibsDir = "/data/data/" + Globals.PackageName;
	    Globals.StorageDir = Globals.DownloadToSdcard ?
                Environment.getExternalStorageDirectory().getAbsolutePath() : getFilesDir().getAbsolutePath();

	    Globals.DataDir = Globals.DownloadToSdcard ?
                Environment.getExternalStorageDirectory().getAbsolutePath() + "/Android/data/" +
                Globals.PackageName : getFilesDir().getAbsolutePath();
         
            Globals.DataDirChecked = true;
            new File( Globals.DataDir + "/data" ).mkdirs();

            gui_cfg = new Config( Globals.DataDir + "/data/gui.cfg" );
            error_log = new Config( Globals.DataDir + "/error.log" );
        }

        String val = gui_cfg.get( "GENERAL", "first_run" );
        if( val == null || val.equals( "1" ) )
        {
            gui_cfg.put( "GENERAL", "first_run", "0" );
            gui_cfg.put( "GAME_PAD", "enabled", "1" );
            gui_cfg.put( "GAME_PAD", "show_fps", "1" );
            gui_cfg.put( "GAME_PAD", "which_pad", "Yabause-AE-Grey" );
            gui_cfg.put( "KEYS", "disable_volume_keys", "0" );
            gui_cfg.put( "Controller1", "plugged", "True" );
            gui_cfg.put( "Controller2", "plugged", "False" );
            for( int x = 1; x < 3; x++ )
            {
                gui_cfg.put( "Controller" + x, "DPad R", "key(0)" );
                gui_cfg.put( "Controller" + x, "DPad L", "key(0)" );
                gui_cfg.put( "Controller" + x, "DPad D", "key(0)" );
                gui_cfg.put( "Controller" + x, "DPad U", "key(0)" );
                gui_cfg.put( "Controller" + x, "Start", "key(0)" );
                gui_cfg.put( "Controller" + x, "A Button", "key(0)" );
                gui_cfg.put( "Controller" + x, "B Button", "key(0)" );
                gui_cfg.put( "Controller" + x, "C Button", "key(0)" );
                gui_cfg.put( "Controller" + x, "X Button", "key(0)" );
                gui_cfg.put( "Controller" + x, "Y Button", "key(0)" );
                gui_cfg.put( "Controller" + x, "Z Button", "key(0)" );
                gui_cfg.put( "Controller" + x, "R Trig", "key(0)" );
                gui_cfg.put( "Controller" + x, "L Trig", "key(0)" );
                gui_cfg.put( "Controller" + x, "X Axis", "key(0,0)" );
                gui_cfg.put( "Controller" + x, "Y Axis", "key(0,0)" );
            }
            gui_cfg.save();
        }

        val = gui_cfg.get( "KEYS", "disable_volume_keys" );
        if( val != null )
            Globals.volumeKeysDisabled = ( val.equals( "1" ) ? true : false );

        List<MenuOption>optionList = new ArrayList<MenuOption>();

        optionList.add( new MenuOption( "Choose Game", "select a game to play", "menuOpenROM" ) );

        biosFile = gui_cfg.get( "LAST_SESSION", "chosen_bios" );
        Globals.chosenBIOS = biosFile;
        if( biosFile != null )
        {
            File f = new File( biosFile );
            biosFile = f.getName();
        }
        if( biosFile == null )
            biosFile = "(not selected)";

        optionList.add( new MenuOption( "Select Bios File", biosFile, "menuOpenBIOS" ) );

        optionList.add( new MenuOption( "Controller 1", "map controller 1 buttons", "menuMapController1" ) );
        optionList.add( new MenuOption( "--todo: controller 2--", "", "line" ) );
//        optionList.add( new MenuOption( "Controller 2", "map controller 2 buttons", "menuMapController2" ) );

        optionList.add( new MenuOption( "Close", "exit the app", "menuClose" ) );

        optionArrayAdapter = new OptionArrayAdapter( this, R.layout.menu_option, optionList );
        setListAdapter( optionArrayAdapter );

        Globals.errorMessage = error_log.get( "OPEN_ROM", "fail_crash" );
        if( Globals.errorMessage != null && Globals.errorMessage.length() > 0 )
        {
            Runnable toastMessager = new Runnable()
            {
                public void run()
                {
                    Toast toast = Toast.makeText( MenuActivity.mInstance, new String( Globals.errorMessage ), Toast.LENGTH_LONG );
                    toast.setGravity( Gravity.BOTTOM, 0, 0 );
                    toast.show();
                }
            };
            this.runOnUiThread( toastMessager );
        }
        error_log.put( "OPEN_ROM", "fail_crash", "" );
        error_log.save();
        Globals.errorMessage = null;
    }
    @Override
    public boolean onKeyDown( int keyCode, KeyEvent event )
    {
        if( keyCode == KeyEvent.KEYCODE_BACK )
            return true;
        return false;
    }
    @Override
    public boolean onKeyUp( int keyCode, KeyEvent event )
    {
        if( keyCode == KeyEvent.KEYCODE_BACK )
            return true;
        return false;
    }
    public void fileChosen( String filename )
    {
        if( filename == null )
        {
            Log.e( "MenuActivity", "filename null in method fileChosen" );
            return;
        }
        Globals.chosenBIOS = filename;
        gui_cfg.put( "LAST_SESSION", "chosen_bios", filename );
        File f = new File( filename );
        String biosName = f.getName();
        if( biosName == null )
        {
            Log.e( "MenuActivity", "bios name null in method fileChosen" );
            return;
        }
        optionArrayAdapter.remove( optionArrayAdapter.getItem( 1 ) );
        optionArrayAdapter.insert( new MenuOption( "Select Bios File", biosName, "menuOpenBIOS" ), 1 );
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
        if( menuOption.info.equals( "menuOpenROM" ) )
        {  // Open the file chooser to pick a ROM
            String path = gui_cfg.get( "LAST_SESSION", "rom_folder" );

            if( path == null || path.length() < 1 )
                FileChooserActivity.startPath = Globals.StorageDir;
            else
                FileChooserActivity.startPath = path;
            FileChooserActivity.extensions = ".iso.cue";
            FileChooserActivity.parentMenu = null;
            Intent intent = new Intent( mInstance, FileChooserActivity.class );
            intent.setFlags( Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP );
            startActivity( intent );
        }
        else if( menuOption.info.equals( "menuOpenBIOS" ) )
        {  // Open the file chooser to pick a ROM
            String path = gui_cfg.get( "LAST_SESSION", "bios_folder" );

            if( path == null || path.length() < 1 )
                FileChooserActivity.startPath = Globals.StorageDir;
            else
                FileChooserActivity.startPath = path;
            FileChooserActivity.extensions = ".bin.rom.bios";
            FileChooserActivity.parentMenu = mInstance;
            Intent intent = new Intent( mInstance, FileChooserActivity.class );
            intent.setFlags( Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP );
            startActivity( intent );
        }
        else if( menuOption.info.equals( "menuMapController1" ) ) 
        {  // Map controller 1 buttons
            MenuSettingsInputConfigureButtonsActivity.controllerNum = 1;
            Intent intent = new Intent( mInstance, MenuSettingsInputConfigureButtonsActivity.class );
            intent.setFlags( Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP );
            startActivity( intent );
        }
        else if( menuOption.info.equals( "menuMapController2" ) ) 
        {  // Map controller 2 buttons
            MenuSettingsInputConfigureButtonsActivity.controllerNum = 2;
            Intent intent = new Intent( mInstance, MenuSettingsInputConfigureButtonsActivity.class );
            intent.setFlags( Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP );
            startActivity( intent );
        }
        else if( menuOption.info.equals( "menuClose" ) ) 
        {  // Shut down the app
            File f = new File( Globals.StorageDir );
            if( f.exists() )
            {
                gui_cfg.save();
            }
            mInstance.finish();
        }
    }
}

