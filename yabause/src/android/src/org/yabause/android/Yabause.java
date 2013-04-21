/*  Copyright 2011 Guillaume Duhamel

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

package org.yabause.android;

import java.lang.Runnable;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;
import android.view.Surface;

class InputHandler extends Handler {
    private YabauseRunnable yr;

    public InputHandler(YabauseRunnable yr) {
        this.yr = yr;
    }

    public void handleMessage(Message msg) {
        if (msg.arg1 == 1) {
            yr.press(msg.arg2);
        } else if (msg.arg1 == 2) {
            yr.release(msg.arg2);
        }
    }
}

class YabauseRunnable implements Runnable
{
    public static native int init(Yabause yabause, Bitmap bitmap);
    public static native void deinit();
    public static native void exec();
    public static native void press(int key);
    public static native void release(int key);
    public static native int initViewport( Surface sf, int width, int hieght);
    public static native int drawScreen();
    public static native int lockGL();
    public static native int unlockGL();
    Yabause _yabause;
    
    private boolean inited;
    private boolean paused;
    public InputHandler handler;

    public YabauseRunnable(Yabause yabause, Bitmap bitmap)
    {
        handler = new InputHandler(this);
        _yabause = yabause;
        int ok = init(_yabause, null);
        inited = (ok == 0);
    }
    
    public void pause()
    {
        Log.v("Yabause", "pause... should really pause emulation now...");
        paused = true;
    }

    public void resume()
    {
        Log.v("Yabause", "resuming emulation...");
        paused = false;
        handler.post(this);
    }

    public void destroy()
    {
        Log.v("Yabause", "destroying yabause...");
        inited = false;
        deinit();
    }

    public void run()
    {
      if (inited && (! paused))
      {
         exec();
      }
    }

    public boolean paused()
    {
        return paused;
    }
}

class YabauseHandler extends Handler {
    private Yabause yabause;

    public YabauseHandler(Yabause yabause) {
        this.yabause = yabause;
    }

    public void handleMessage(Message msg) {
        yabause.showDialog(msg.what, msg.getData());
    }
}

public class Yabause extends Activity
{
    private static final String TAG = "Yabause";
    private YabauseRunnable yabauseThread;
    private YabauseHandler handler;
    public static int width, height;
    public static Yabause mSingleton = null;

    // Virtual gamepad
    public static GamePad mGamePad = null;
    public static GamePad.GamePadListing mGamePadListing = null;
    public static int whichPad = 0;
    public static boolean[] previousButtonStates = new boolean[13];
    // todo: implement multi-controller and analog

    private static NotificationManager notificationManager = null;
    // Toast Messages:
    private static Toast toast = null;
    private static Runnable toastMessager = null;

    private static int frameCount = -1;
    private static int fpsRate = 15;
    private static long lastFPSCheck = 0;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        // paulscode, place an icon into the status bar:
        if( notificationManager == null )
            notificationManager = (NotificationManager) getSystemService( Context.NOTIFICATION_SERVICE );
        int statusIcon = R.drawable.status;
        CharSequence text = "Yabause is running";
        CharSequence contentTitle = "Yabause";
        CharSequence contentText = "Yabause";
        long when = System.currentTimeMillis();
        Context context = getApplicationContext();

        Intent intent = new Intent( this, MenuActivity.class );
        intent.setFlags( Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP );
        PendingIntent contentIntent = PendingIntent.getActivity( this, 0, intent, 0 );
        Notification notification = new Notification( statusIcon, text, when );
        notification.flags = Notification.FLAG_ONGOING_EVENT | Notification.FLAG_NO_CLEAR;
        notification.setLatestEventInfo( context, contentTitle, contentText, contentIntent );
        notificationManager.notify( Globals.NOTIFICATION_ID, notification );
        super.onCreate(savedInstanceState);
        requestWindowFeature( Window.FEATURE_NO_TITLE );
        getWindow().setFlags( WindowManager.LayoutParams.FLAG_FULLSCREEN,
                              WindowManager.LayoutParams.FLAG_FULLSCREEN );
        getWindow().setFlags( WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                              WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );

        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics( metrics );
        if( metrics.widthPixels > metrics.heightPixels )
        {
            width = metrics.widthPixels;
            height = metrics.heightPixels;
        }
        else
        {
            width = metrics.heightPixels;
            height = metrics.widthPixels;
        }
        Globals.populateControls();

        for( int x = 0; x < 13; x++ )
        {
            previousButtonStates[x] = false;
        }

        setContentView(R.layout.main);

        YabauseView view = (YabauseView) findViewById(R.id.yabause_view);
        handler = new YabauseHandler(this);
        yabauseThread = new YabauseRunnable(this,null);
        //view.setYabauseRunnable(yabauseThread);

        mSingleton = this;

        mGamePad = (GamePad) findViewById( R.id.yabause_pad );
        mGamePad.setResources( getResources() );
        mGamePadListing = new GamePad.GamePadListing( Globals.DataDir + "/skins/gamepads/gamepad_list.ini" );

        // make sure the gamepad preferences are loaded;
        String val = MenuActivity.gui_cfg.get( "GAME_PAD", "show_fps" );
        if( val != null )
            Globals.showFPS = ( val.equals( "1" ) ? true : false );
        val = MenuActivity.gui_cfg.get( "GAME_PAD", "enabled" );
        if( val != null )
            Globals.gamepadEnabled = ( val.equals( "1" ) ? true : false );
        Globals.chosenGamepad = MenuActivity.gui_cfg.get( "GAME_PAD", "which_pad" );

        if( !Globals.gamepadEnabled )
            mGamePad.loadPad( null );
        else if( Globals.chosenGamepad != null && Globals.chosenGamepad.length() > 0 )
            mGamePad.loadPad( Globals.chosenGamepad );
        else if( mGamePadListing.numPads > 0 )
            mGamePad.loadPad( mGamePadListing.padNames[0] );
        else
        {
            mGamePad.loadPad( null );
            Log.v( "Yabause", "No gamepad skins found" );
        }
            
        showToast( "Yabause Started" );
    }

    @Override
    public void onPause()
    {
        super.onPause();
        Log.v(TAG, "pause... should pause emulation...");
        yabauseThread.pause();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        Log.v(TAG, "resume... should resume emulation...");
        yabauseThread.resume();
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
        Log.v(TAG, "this is the end...");
        yabauseThread.destroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.emulation, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.pause:
            yabauseThread.pause();
            return true;
        case R.id.quit:
            if( notificationManager != null )
                notificationManager.cancel( Globals.NOTIFICATION_ID );
            this.finish();
            return true;
        case R.id.resume:
            yabauseThread.resume();
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (yabauseThread.paused()) {
            menu.setGroupVisible(R.id.paused, true);
            menu.setGroupVisible(R.id.running, false);
        } else {
            menu.setGroupVisible(R.id.paused, false);
            menu.setGroupVisible(R.id.running, true);
        }
        return true;
    }

    @Override
    public Dialog onCreateDialog(int id, Bundle args) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(args.getString("message"))
            .setCancelable(false)
            .setNegativeButton("Exit", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    Yabause.this.finish();
                }
            })
            .setPositiveButton("Ignore", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    dialog.cancel();
                }
            });
        AlertDialog alert = builder.create();
        return alert;
    }

    public static void keyDown( int code )
    {
        if( mSingleton == null )
            return;

        int val = getButtonVal( code );
        if( val > -1 )
            mSingleton.buttonAction( 1, val );
    }
    public static void keyUp( int code )
    {
        if( mSingleton == null )
            return;

        int val = getButtonVal( code );
        if( val > -1 )
            mSingleton.buttonAction( 2, val );
    }

    public static int getButtonVal( int code )
    {
        if( code == 0 )
            return -1;
        // TODO: implement controller 2
        int x = 0;
        for( int y = 0; y < 13; y++ )
        {
            if( Globals.ctrlr[x][y] == code )
                return y;
        }
        return -1;
    }

    public static void updateVirtualGamePadStates( boolean[] buttons )
    {
        if( mSingleton == null )
            return;
        for( int x = 0; x < 13; x++ )
        {
            if( buttons[x] && !previousButtonStates[x] )
                mSingleton.buttonAction( 1, x );
            else if( !buttons[x] && previousButtonStates[x] )
                mSingleton.buttonAction( 2, x );
            previousButtonStates[x] = buttons[x];
        }
    }

    public void buttonAction( int action, int val )
    {
        Message message = handler.obtainMessage();
        message.arg1 = action;
        message.arg2 = val;
        yabauseThread.handler.sendMessage(message);
    }

    private void errorMsg(String msg) {
        Message message = handler.obtainMessage();
        Bundle bundle = new Bundle();
        bundle.putString("message", msg);
        message.setData(bundle);
        handler.sendMessage(message);
    }
    public static Object getCDImage()
    {
        return (Object) Globals.chosenROM;
    }
    public static Object getBios()
    {
        return (Object) Globals.chosenBIOS;
    }

    public static void showToast( String message )
    {
        if( mSingleton == null )
            return;
        if( toast != null )
            toast.setText( new String( message ) );
        else
        {
            toast = Toast.makeText( mSingleton, new String( message ), Toast.LENGTH_SHORT );
            toast.setGravity( Gravity.BOTTOM, 0, 0 );
        }
        // Toast messages must be run on the UiThread, which looks ugly as hell, but works:
        if( toastMessager == null )
            toastMessager = new Runnable()
                            {
                                public void run()
                                {
                                    if( toast != null )
                                        toast.show();
                                }
                            };
        mSingleton.runOnUiThread( toastMessager );
    }
    public static void countFrame()
    {
        if( frameCount < 0 )
        {
            frameCount = 0;
            lastFPSCheck = System.currentTimeMillis();
        }
        frameCount++;
        if( (mGamePad != null && frameCount >= mGamePad.fpsRate) ||
            (mGamePad == null && frameCount >= fpsRate) )
        {
            long currentTime = System.currentTimeMillis();
            float fFPS = ( (float) frameCount / (float) (currentTime - lastFPSCheck) ) * 1000.0f;
            if( mGamePad != null )
                mGamePad.updateFPS( (int) fFPS );
            frameCount = 0;
            lastFPSCheck = currentTime;
        }
    }

    static {
        System.loadLibrary("yabause");
    }
}
