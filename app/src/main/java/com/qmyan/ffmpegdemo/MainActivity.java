package com.qmyan.ffmpegdemo;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.qmyan.ffmpegdemo.utils.FileUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import static android.media.AudioFormat.ENCODING_PCM_16BIT;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "qmyan";
    private static final int REQUEST_CODE_OPEN_FILE = 1;

    private EditText mSrcText;
    private TextView tv;
    private Handler mHandler = new Handler(){
        @Override
        public void handleMessage(@NonNull Message msg) {
            if (msg.what == 0x123) {
                tv.append("Demuxing and decoding completed!\nAnd you can start to play!");
            }
            super.handleMessage(msg);
        }
    };

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private boolean isPlaying = false;
    private boolean isStop = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        mSrcText = findViewById(R.id.src_text);
        tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
//        tv.setText("Hello World");
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native void demuxing_decoding(String src, String audioOutputFile, String videoOutputFile);

    public void stopPlayMusic(View view) {
        stopPlay();
        tv.setText("stop done!");
    }

    public void startPlayMusic(View view) {
        tv.setText("playing......");
        new Thread(new Runnable() {
            @Override
            public void run() {
                String audiofilepath = Environment.getExternalStorageDirectory().getAbsolutePath()
                        + File.separator + "Download" + File.separator + "out.pcm";
                int bufferSize = AudioTrack.getMinBufferSize(22050, AudioFormat.CHANNEL_OUT_STEREO, ENCODING_PCM_16BIT);
                AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, 22050, AudioFormat.CHANNEL_OUT_STEREO,ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
                FileInputStream fis = null;
                try {
                    audioTrack.play();
                    fis = new FileInputStream(audiofilepath);
                    byte[] buffer = new byte[bufferSize];
                    int len = 0;
                    isPlaying = true;
                    while ((len = fis.read(buffer)) != -1 && !isStop) {
                        Log.d(TAG, "playPCMRecord: len " + len);
                        audioTrack.write(buffer, 0, len);
                    }

                } catch (Exception e) {
                    Log.e(TAG, "playPCMRecord: e : " + e);
                } finally {
                    isPlaying = false;
                    isStop = false;
                    if (audioTrack != null) {
                        audioTrack.stop();
                        audioTrack.release();
                        audioTrack = null;
                    }
                    Log.d(TAG, "stop done");
                    if (fis != null) {
                        try {
                            fis.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }

            }
        }).start();
    }

    public void demuxingAnddecode(View view) {
        stopPlay();
        tv.setText("Demuxing and decoding......\nPlease wait!\n");
        new Thread(new Runnable() {
            @Override
            public void run() {
                String src = mSrcText.getText().toString();
                if (src.isEmpty()) {
                    src = Environment.getExternalStorageDirectory().getAbsolutePath()
                            + File.separator + "Download" + File.separator + "jayzhou.mp4";
                }
                String audiofilepath = Environment.getExternalStorageDirectory().getAbsolutePath()
                        + File.separator + "Download" + File.separator + "out.pcm";
                Log.d(TAG, "src->" + src);
                Log.d(TAG, "audiofilepath->" + audiofilepath);
                demuxing_decoding(src, audiofilepath, "");
                Message message = mHandler.obtainMessage();
                message.what = 0x123;
                mHandler.sendMessage(message);
            }
        }).start();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
        stopPlay();
    }

    private void stopPlay() {
        if (isPlaying) {
            isStop = true;
        }
    }

    public void openFile(View view) {
        tv.setText("");
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent .setType("*/*");
        String[] mimeTypes = {"audio/*", "video/*"}; // 同时选择视频和音频
        intent .putExtra(Intent.EXTRA_MIME_TYPES, mimeTypes);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(intent,REQUEST_CODE_OPEN_FILE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        if (REQUEST_CODE_OPEN_FILE == requestCode) {
            if (resultCode == Activity.RESULT_OK) {
                String path;
                Uri uri = data.getData();
                Log.d(TAG, "onActivityResult: Uri -> " + uri.toString());
                Log.d(TAG, "onActivityResult: Scheme ->  " + uri.getScheme());
                Log.d(TAG, "onActivityResult: Build.VERSION ? 19 -> " + Build.VERSION.SDK_INT);
                if (Build.VERSION.SDK_INT > Build.VERSION_CODES.KITKAT) {   // 4.4以后
                    path = FileUtils.getPath(this, uri);
                    mSrcText.setText(path);
                    return;
                } else {    //4.4 以下下系统调用方法
                    path = FileUtils.getRealPathFromURI(this, uri);
                    mSrcText.setText(path);
                    return;
                }
            }
        }
        super.onActivityResult(requestCode, resultCode, data);
    }
}
