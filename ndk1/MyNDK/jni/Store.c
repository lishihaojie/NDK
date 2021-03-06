/*
 * Store.c
 *
 *  Created on: May 12, 2015
 *      Author: Lioncraft
 */
#include "Store.h"
#include <string.h>

int32_t isEntryValid(JNIEnv* pEnv, StoreEntry* pEntry, StoreType type) {
	if (NULL == pEntry) {
		throwNotExistingKeyException(pEnv);
	} else if (pEntry->mType != type) {
		throwInvalidTypeException(pEnv);
	} else {
		return 1;
	}

	// We arrive here if an exception is raised (raising an
	// exception does not stop the flow of code like in Java).
	return 0;
}

StoreEntry* findEntry(JNIEnv* pEnv, Store* pStore, jstring jstrKey,
		int32_t* pError) {
	StoreEntry* lEntry = pStore->mEntries;
	StoreEntry* lEntryEnd = lEntry + pStore->mLength;
	// Compare requested key with every entry key currently stored
	// until we find a matching one.
	const char * lKeyTmp = (*pEnv)->GetStringUTFChars(pEnv, jstrKey, NULL);

	if (NULL == lKeyTmp) {
		if (NULL != pError) {
			*pError = 1;
		}

		return NULL;
	}

	while ((lEntry < lEntryEnd) && (strcmp(lEntry->mKey, lKeyTmp) != 0)) {
		++lEntry;
	}

	(*pEnv)->ReleaseStringUTFChars(pEnv, jstrKey, lKeyTmp);

	return (lEntry == lEntryEnd) ? NULL : lEntry;

}

StoreEntry * allocateEntry(JNIEnv* pEnv, Store* pStore, jstring jstrKey) {
	// If entry already exists in the store, releases its content
	// and keep its key.
	int32_t lError = 0;
	StoreEntry* lEntry = findEntry(pEnv, pStore, jstrKey, &lError);
	if (NULL != lEntry) {
		releaseEntryValue(pEnv, lEntry);
	}
	// If entry does not exist, create a new entry right after
	// already allocated entries.
	else if (!lError) {
		// Checks store can accept a new entry.
		if (pStore->mLength >= STORE_MAX_CAPACITY) {
			throwStoreFullException(pEnv);
			return NULL;
		}

		lEntry = pStore->mEntries + pStore->mLength;

		// Copies the new key into its final C string buffer.
		const char* lKeyTmp = (*pEnv)->GetStringUTFChars(pEnv, jstrKey, NULL);

		if (NULL == lKeyTmp) {
			return NULL;
		}

		lEntry->mKey = (char*) malloc(strlen(lKeyTmp));
		strcpy(lEntry->mKey, lKeyTmp);
		(*pEnv)->ReleaseStringUTFChars(pEnv, jstrKey, lKeyTmp);

		++pStore->mLength;
	}

	return lEntry;
}

void releaseEntryValue(JNIEnv* pEnv, StoreEntry* pEntry) {
	int32_t i;
	switch (pEntry->mType) {
	case StoreType_String:
		free(pEntry->mValue.mString);
		break;
	case StoreType_Color:
		// Unreferences the Id object for garbage collection.
		(*pEnv)->DeleteGlobalRef(pEnv, pEntry->mValue.mColor);
		break;
	case StoreType_BooleanArray:
		free(pEntry->mValue.mBooleanArray);
		break;
	case StoreType_ByteArray:
		free(pEntry->mValue.mByteArray);
		break;
	case StoreType_CharArray:
		free(pEntry->mValue.mCharArray);
		break;
	case StoreType_DoubleArray:
		free(pEntry->mValue.mDoubleArray);
		break;
	case StoreType_FloatArray:
		free(pEntry->mValue.mDoubleArray);
		break;
	case StoreType_IntegerArray:
		free(pEntry->mValue.mIntegerArray);
		break;
	case StoreType_LongArray:
		free(pEntry->mValue.mLongArray);
		break;
	case StoreType_ShortArray:
		free(pEntry->mValue.mShortArray);
		break;
	case StoreType_StringArray:
		// Destroys every C string pointed by the String array
		// before releasing it.
		for (i =0; i < pEntry->mLength; ++i) {
			free(pEntry->mValue.mStringArray[i]);
		}
		free(pEntry->mValue.mStringArray);
		break;
	case StoreType_ColorArray:
		//// Unreferences every Id before releasing the Id array.

		for (i = 0; i < pEntry->mLength; ++i) {
			(*pEnv)->DeleteWeakGlobalRef(pEnv, pEntry->mValue.mColorArray[i]);
		}
		free(pEntry->mValue.mColorArray);
		break;
	}
}

void throwNotExistingKeyException(JNIEnv* pEnv)
{
	jclass lClass = (*pEnv)->FindClass(pEnv, "com/example/exception/NotExistingKeyException");

	if (NULL != lClass) {
		(*pEnv)->ThrowNew(pEnv, lClass, "Key does not exist.");
	}
	(*pEnv)->DeleteLocalRef(pEnv, lClass);
}

void throwInvalidTypeException(JNIEnv* pEnv)
{
    jclass lClass = (*pEnv)->FindClass(pEnv,
        "com/example/exception/InvalidTypeException");
    if (NULL != lClass) {
        (*pEnv)->ThrowNew(pEnv, lClass, "Type is invalid.");
    }
    (*pEnv)->DeleteLocalRef(pEnv, lClass);
}

void throwStoreFullException(JNIEnv* pEnv)
{
    jclass lClass = (*pEnv)->FindClass(pEnv,
        "com/example/exception/StoreFullException");
    if (NULL != lClass) {
        (*pEnv)->ThrowNew(pEnv, lClass, "Store is full.");
    }
    (*pEnv)->DeleteLocalRef(pEnv, lClass);
}

