package com.entityname.cssv34;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.util.Locale;
import java.io.File;
import android.os.Environment;
import android.content.Intent;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.widget.Button;
import android.widget.TextView;

import androidx.core.app.ActivityCompat;

import org.libsdl.app.SDLActivity;
import org.w3c.dom.Text;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("SDL2");
        System.loadLibrary("launcher");
    }

    public static native int setenv(String name, String value, int overwrite);
    public static native void setArgs(String args);

    static private void copyFonts(Context ctx)
    {
        AssetManager assetManager = ctx.getAssets();
        try {
            String[] assetFiles = assetManager.list("");
            if (assetFiles == null) {
                return;
            }

            File destDir = ctx.getFilesDir();
            Log.v("ASSETS", destDir.toString());

            if (!destDir.exists())
            {
                if (!destDir.mkdirs())
                {
                    return;
                }
            }

            for (String fileName : assetFiles) {
                if (!fileName.toLowerCase().endsWith(".ttf")) {
                    continue;
                }
                File destFile = new File(destDir, fileName);
                if (!destFile.exists()) {
                    continue;
                }
                try (InputStream in = assetManager.open(fileName); OutputStream out = new FileOutputStream(destFile)) {
                    byte[] buffer = new byte[8192];
                    int read;
                    while ((read = in.read(buffer)) != -1)
                    {
                        out.write(buffer, 0, read);
                    }
                    Log.v("ASSETS", "Copied file "+fileName+" -> "+destFile.getPath());
                }
                catch (IOException e)
                {
                    Log.v("ASSETS", "Failed to copy file "+fileName+": "+e.toString());
                }
            }
        }
        catch (IOException e)
        {
            Log.v("ASSETS", "Failed to copy files: "+e.toString());
        }
    }

    public static String getDefaultDir() {
        File dir = Environment.getExternalStorageDirectory();
        Log.v("cssv34", "Dir:"+dir);
        if (dir == null || !dir.exists())
            return "/sdcard/";
        return dir.getPath();
    }

    static public void initNatives(Context context, Intent intent) {
        ExtractAssets.extractVPK(context);
        String vpks = context.getFilesDir().getPath()+"/"+ExtractAssets.VPK_NAME;

        ApplicationInfo appinf = context.getApplicationInfo();
        String gamepath = getDefaultDir() + "/srceng";

        String argv = "-console";
        String gamedir = "cstrike";

        argv = "-game " + gamedir + " " + argv;

        setenv( "LANG", Locale.getDefault().toString(), 1 );
        setenv( "APP_DATA_PATH", appinf.dataDir, 1);
        setenv( "APP_DATA_PATH", appinf.nativeLibraryDir, 1);
        setenv( "EXTRAS_VPK_PATH", vpks, 1);

        setenv( "VALVE_GAME_PATH", gamepath, 1 );

        setArgs(argv);
        copyFonts(context);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        if (hasFocus) {
            hideSystemUi();
        }
    }

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(1);

        setContentView(R.layout.activity_main);

        Button button = (Button)findViewById(R.id.launch);
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                startSource(v);
            }
        });

        if (Environment.isExternalStorageManager())
        {
            button.setText("Start game");
        }
        else
        {
            TextView view = (TextView)findViewById(R.id.textView);
            view.setText("No 'manage app all files access' permission");
            button.setText("Request it");
        }
    }

    public void startSource(View view)
    {
        if (Environment.isExternalStorageManager())
        {
            Intent intent = new Intent(MainActivity.this, SDLActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
        else {
            Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
            Uri uri = Uri.fromParts("package", getPackageName(), null);
            intent.setData(uri);
            startActivity(intent);
        }
    }

    private void hideSystemUi() {
        WindowInsetsController insetsController = getWindow().getInsetsController();
        if (insetsController != null) {
            insetsController.hide(WindowInsets.Type.systemBars());
            insetsController.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        }
    }
}