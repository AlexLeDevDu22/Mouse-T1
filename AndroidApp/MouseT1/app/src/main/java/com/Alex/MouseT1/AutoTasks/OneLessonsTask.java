package com.Alex.MouseT1.AutoTasks;

import static com.Alex.MouseT1.OtherFun.readTextFromFile;
import static com.Alex.MouseT1.OtherFun.sendLargeJson;

import android.app.Activity;
import android.os.Build;
import android.util.Log;

import androidx.annotation.RequiresApi;


import java.io.File;

public class OneLessonsTask extends Activity {
    @RequiresApi(api = Build.VERSION_CODES.O)
    public void startTask(Activity activity, String title, String postUrl) {
        try {

            String lesson = getLessonByTitle(activity, title);

            String postLessons;
            if (lesson != null) {
                postLessons = "{\"type\":\"text\",\"text\":\"" + lesson + "\"}";

            } else {
                postLessons = "{\"type\":\"text\",\"text\":\"Lesson not found\"}";
            }
            if (postLessons.length()>2048){
                postLessons="{\"type\":\"text\",\"text\":\"Erreur: leçon trop longue!\"}";
            }
            sendLargeJson(postLessons, postUrl);
        }catch(Exception e){
            Log.e("testaa",e.toString());
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private String getLessonByTitle(Activity activity ,String title) {
        try {
            String Lessons=null;

            File filesDir = activity.getExternalFilesDir(null);
            File[] files = filesDir != null ? filesDir.listFiles((dir, name) -> name.endsWith(".txt")) : null;

            if (files != null) {
                for (File file : files) {
                    if (file.getName().replaceAll(".txt$","").equals(title)) {
                        Lessons=readTextFromFile(file).replaceAll("\n+$", "");
                    }
                }
                return Lessons;
            }
        }catch (Exception e) {
            Log.e("testaa",e.toString());
        }
        return null;
    }
}