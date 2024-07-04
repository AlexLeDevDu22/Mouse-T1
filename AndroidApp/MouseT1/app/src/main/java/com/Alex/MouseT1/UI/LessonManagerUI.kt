package com.Alex.MouseT1.UI

import android.annotation.SuppressLint
import android.app.AlertDialog
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.OpenableColumns
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.widget.*
import androidx.annotation.Nullable
import androidx.appcompat.app.AppCompatActivity
import com.Alex.MouseT1.Lesson
import com.Alex.MouseT1.R
import java.io.*
import java.nio.charset.Charset
import java.text.SimpleDateFormat
import java.util.*


class LessonManagerUI : AppCompatActivity() {

    private lateinit var lessonListContainer: LinearLayout
    private lateinit var addFileButton: Button
    private val FILE_SELECT_CODE = 0
    private lateinit var selectedSubject: String
    private lateinit var selectedFileName: String
    private lateinit var selectedFileUri: Uri

    // Liste pour stocker les leçons chargées
    private val lessonList = mutableListOf<Lesson>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.lesson_manager_ui)

        lessonListContainer = findViewById(R.id.lessonListContainer)
        addFileButton = findViewById(R.id.addFileButton)

        loadFiles()

        addFileButton.setOnClickListener {
            openFileChooser()
        }
    }

    private fun loadFiles() {
        val filesDir = getExternalFilesDir(null)
        val files = filesDir?.listFiles { file ->
            file.extension == "txt"
        }

        files?.forEach { file ->
            val metadataFile = File(file.path.replace(".txt", ".meta"))
            val subject = if (metadataFile.exists()) {
                metadataFile.readText()
            } else {
                "Inconnue"
            }

            val lesson = Lesson(
                name = file.name,
                subject = subject,
                date = file.lastModified(),
                text = readTextFromFile(file),
                filePath = file.path  // Utilisation du chemin d'accès au fichier
            )
            lessonList.add(lesson) // Ajouter la leçon à la liste
            val lessonView = createLessonView(lesson)
            lessonListContainer.addView(lessonView)
        }
    }

    private fun readTextFromFile(file: File): String {
        val stringBuilder = StringBuilder()
        var bufferedReader: BufferedReader? = null

        try {
            bufferedReader = BufferedReader(FileReader(file))
            var line: String?

            while (bufferedReader.readLine().also { line = it } != null) {
                stringBuilder.append(line).append("\n")
            }
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            try {
                bufferedReader?.close()
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }

        return stringBuilder.toString()
    }

    @SuppressLint("SetTextI18n")
    private fun createLessonView(lesson: Lesson): View {
        val inflater = LayoutInflater.from(this)
        val lessonView = inflater.inflate(R.layout.item_lesson, lessonListContainer, false)

        val fileNameTextView = lessonView.findViewById<TextView>(R.id.fileNameTextView)
        val fileSubjectTextView = lessonView.findViewById<TextView>(R.id.fileSubjectTextView)
        val fileDateTextView = lessonView.findViewById<TextView>(R.id.fileDateTextView)
        val deleteButton = lessonView.findViewById<Button>(R.id.deleteButton)
        val openButton = lessonView.findViewById<Button>(R.id.openButton)

        fileNameTextView.text = lesson.name
        fileSubjectTextView.text = "Matière: ${lesson.subject}"
        fileDateTextView.text = "Date: " + SimpleDateFormat("dd/MM/yyyy", Locale.getDefault()).format(lesson.date)

        deleteButton.setOnClickListener {
            // Supprimer la leçon de la liste et de l'UI
            lessonList.remove(lesson)
            lessonListContainer.removeView(lessonView)
            // Supprimer le fichier du stockage externe
            deleteLessonFile(lesson)
        }

        openButton.setOnClickListener {
            openLessonTextActivity(lesson)
        }

        return lessonView
    }

    private fun deleteLessonFile(lesson: Lesson) {
        val file = File(lesson.filePath)
        val metadataFile = File(lesson.filePath.replace(".txt", ".meta"))
        if (file.exists()) {
            file.delete()
        }
        if (metadataFile.exists()) {
            metadataFile.delete()
        }
    }

    private fun openLessonTextActivity(lesson: Lesson) {
        val intent = Intent(this, LessonTextActivity::class.java)
        intent.putExtra("lessonName", lesson.name)
        intent.putExtra("lessonSubject", lesson.subject)
        intent.putExtra("lessonDate", lesson.date)
        intent.putExtra("lessonText", lesson.text)
        startActivity(intent)
    }

    private fun showAddFileDialog(defaultFileName: String) {
        val dialogView = LayoutInflater.from(this).inflate(R.layout.dialog_add_file, null)
        val subjectSpinner = dialogView.findViewById<Spinner>(R.id.subjectSpinner)
        val fileNameEditText = dialogView.findViewById<EditText>(R.id.fileNameEditText)

        val subjects = arrayOf("Français", "Histoire", "Géographie", "Anglais", "Espagnol", "EMC", "Sciences", "Mathématiques", "NSI", "Physique-chimie")
        val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, subjects)
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        subjectSpinner.adapter = adapter

        fileNameEditText.setText(defaultFileName.substring(0, defaultFileName.length - 4))

        val dialog = AlertDialog.Builder(this)
            .setTitle("Ajouter un fichier")
            .setView(dialogView)
            .setPositiveButton("Suivant", null)
            .setNegativeButton("Annuler", null)
            .create()

        dialog.setOnShowListener {
            val button = dialog.getButton(AlertDialog.BUTTON_POSITIVE)
            button.setOnClickListener {
                val subject = subjectSpinner.selectedItem.toString()
                val fileName = fileNameEditText.text.toString()

                // Vérifier si le nom est vide ou déjà utilisé
                if (fileName.isBlank()) {
                    fileNameEditText.error = "Le nom du fichier ne peut pas être vide"
                    return@setOnClickListener
                }
                if (lessonList.any { it.name == fileName } or lessonList.any { it.name.replace(".txt","") == fileName }) {
                    fileNameEditText.error = "Une leçon avec ce nom existe déjà"
                    return@setOnClickListener
                }

                selectedSubject = subject
                selectedFileName = fileName
                saveLessonTextFromUri(selectedFileUri, addFileExtension(selectedFileName, selectedFileUri))
                dialog.dismiss()
            }
        }

        dialog.show()
    }

    private fun addFileExtension(fileName: String, uri: Uri): String {
        val contentResolver = contentResolver
        val mimeType = contentResolver.getType(uri)
        val extension = when (mimeType) {
            "text/plain" -> ".txt"
            else -> ""
        }
        return if (fileName.endsWith(extension)) fileName else "$fileName$extension"
    }

    private fun saveLessonTextFromUri(uri: Uri, fileName: String) {
        val inputStream: InputStream? = contentResolver.openInputStream(uri)
        val text = inputStream?.bufferedReader().use { it?.readText() ?: "" }
        inputStream?.close()

        // Save the file to external storage
        val filesDir = getExternalFilesDir(null)
        val file = File(filesDir, fileName)
        file.writeText(text, Charset.defaultCharset())

        // Save the metadata (subject)
        val metadataFile = File(filesDir, fileName.replace(".txt", ".meta"))
        metadataFile.writeText(selectedSubject)

        Log.d("testaa", "texte: $text yooo")

        // Enregistrer les détails de la leçon dans une base de données ou autre méthode de stockage
        val lesson = Lesson(
            name = fileName,
            subject = selectedSubject,
            date = System.currentTimeMillis(),
            text = text,  // Stocker le texte extrait dans l'objet Lesson
            filePath = file.path  // Stocker le chemin d'accès au fichier
        )

        // Ajouter la leçon à la liste et mettre à jour l'UI
        lessonList.add(lesson)
        val lessonView = createLessonView(lesson)
        lessonListContainer.addView(lessonView)
    }

    private fun generateDefaultFileName(uri: Uri): String {
        val contentResolver = contentResolver
        val cursor = contentResolver.query(uri, null, null, null, null)
        val nameIndex = cursor?.getColumnIndex(OpenableColumns.DISPLAY_NAME)
        cursor?.moveToFirst()
        val name = nameIndex?.let { cursor.getString(it) }
        cursor?.close()
        return name ?: "Nouvelle Leçon"
    }

    private fun openFileChooser() {
        val intent = Intent(Intent.ACTION_GET_CONTENT)
        intent.type = "text/plain"
        startActivityForResult(Intent.createChooser(intent, "Select a text file"), FILE_SELECT_CODE)
    }

    @Deprecated("This method has been deprecated in favor of using the Activity Result API\n      which brings increased type safety via an {@link ActivityResultContract} and the prebuilt\n      contracts for common intents available in\n      {@link androidx.activity.result.contract.ActivityResultContracts}, provides hooks for\n      testing, and allow receiving results in separate, testable classes independent from your\n      activity. Use\n      {@link #registerForActivityResult(ActivityResultContract, ActivityResultCallback)}\n      with the appropriate {@link ActivityResultContract} and handling the result in the\n      {@link ActivityResultCallback#onActivityResult(Object) callback}.")
    public override fun onActivityResult(
        requestCode: Int,
        resultCode: Int,
        @Nullable data: Intent?,
    ) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == FILE_SELECT_CODE && resultCode == RESULT_OK) {
            val uri = data?.data
            if (uri != null) {
                selectedFileUri = uri

                // Lire le contenu du fichier et vérifier sa longueur
                val fileContent = readTextFromUri(uri)
                if (fileContent.length > 2000) {
                    // Afficher un message d'erreur si le fichier dépasse 2000 caractères
                    Toast.makeText(
                        this,
                        "Le fichier dépasse la limite de 2000 caractères",
                        Toast.LENGTH_SHORT
                    ).show()
                    return  // Ne pas poursuivre l'ajout du fichier
                }

                val defaultFileName = generateDefaultFileName(uri)
                showAddFileDialog(defaultFileName)
            }
        }
    }

    private fun readTextFromUri(uri: Uri): String {
        val stringBuilder = java.lang.StringBuilder()
        try {
            contentResolver.openInputStream(uri).use { inputStream ->
                BufferedReader(InputStreamReader(inputStream)).use { reader ->
                    var line: String?
                    while ((reader.readLine().also { line = it }) != null) {
                        stringBuilder.append(line).append("\n")
                    }
                }
            }
        } catch (e: IOException) {
            e.printStackTrace()
        }
        return stringBuilder.toString()
    }

}
