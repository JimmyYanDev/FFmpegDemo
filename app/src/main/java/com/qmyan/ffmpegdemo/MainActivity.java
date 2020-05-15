package com.qmyan.ffmpegdemo;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "qmyan";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI());
        tv.setText("Hello World");
    }

    @Override
    protected void onResume() {
        super.onResume();
        new Thread(new Runnable() {
            @Override
            public void run() {
                String src = Environment.getExternalStorageDirectory().getAbsolutePath()
//                        + File.separator + "Movies" + File.separator + "xiaolajiao.mp4";
                        + File.separator + "xiaolajiao.mp4";
                String audiofilepath = Environment.getExternalStorageDirectory().getAbsolutePath()
                        + File.separator + "Music" + File.separator + "xiaolajiao.mp3";
                Log.d(TAG, "src->" + src);
                Log.d(TAG, "audiofilepath->" + audiofilepath);
                demuxing_decoding(src, audiofilepath, "");
            }
        }).start();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native void demuxing_decoding(String src, String audioOutputFile, String videoOutputFile);
}
