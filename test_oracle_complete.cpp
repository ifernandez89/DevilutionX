/**
 * @file test_oracle_complete.cpp
 * 
 * 🔮 TEST COMPLETO DEL SISTEMA DEL ORÁCULO
 * 
 * Prueba el sistema completo:
 * - Textos dormidos
 * - Construcción de prompts
 * - Query a Ollama
 * - Validación de respuestas
 * - Cache
 * 
 * COMPILAR:
 * g++ -std=c++17 test_oracle_complete.cpp \
 *     Source/oracle/oracle_dormant_texts.cpp \
 *     Source/oracle/oracle_prompt.cpp \
 *     Source/oracle/oracle_ollama.cpp \
 *     Source/oracle/oracle_validator.cpp \
 *     Source/oracle/oracle_cache.cpp \
 *     Source/utils/str_cat.cpp \
 *     -I. -ISource \
 *     -lwinhttp -lSDL2 \
 *     -o test_oracle_complete.exe
 */

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <vector>

#include "Source/oracle/oracle_dormant_texts.h"
#include "Source/oracle/oracle_prompt.h"
#include "Source/oracle/oracle_ollama.h"
#include "Source/oracle/oracle_validator.h"
#include "Source/oracle/oracle_cache.h"
#include "Source/oracle/oracle_system.h"

using namespace devilution;

// ============================================================================
// 🔮 UTILIDADES DE TEST
// ============================================================================

void PrintSeparator(const std::string& title = "") {
	std::cout << "\n";
	std::cout << "================================================================\n";
	if (!title.empty()) {
		std::cout << "  " << title << "\n";
		std::cout << "================================================================\n";
	}
}

void PrintSuccess(const std::string& message) {
	std::cout << "✅ " << message << "\n";
}

void PrintError(const std::string& message) {
	std::cout << "❌ " << message << "\n";
}

void PrintInfo(const std::string& message) {
	std::cout << "ℹ️  " << message << "\n";
}

void WaitForResponse(bool& received, int maxSeconds = 10) {
	std::cout << "⏳ Esperando respuesta";
	for (int i = 0; i < maxSeconds && !received; ++i) {
		std::cout << "." << std::flush;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	std::cout << "\n";
}

// ============================================================================
// 🔮 TESTS
// ============================================================================

bool Test1_OllamaAvailability() {
	PrintSeparator("TEST 1: Verificar Disponibilidad de Ollama");
	
	std::cout << "Verificando si Ollama está corriendo...\n";
	bool available = OracleOllama::IsAvailable();
	
	if (available) {
		PrintSuccess("Ollama está disponible en localhost:11434");
		return true;
	} else {
		PrintError("Ollama NO está disponible");
		PrintInfo("Inicia Ollama con: ollama serve");
		return false;
	}
}

bool Test2_DormantTexts() {
	PrintSeparator("TEST 2: Textos Dormidos");
	
	std::cout << "Verificando textos dormidos...\n";
	
	if (!OracleDormantTexts::HasDormantTexts()) {
		PrintError("No hay textos dormidos disponibles");
		return false;
	}
	
	size_t count = OracleDormantTexts::GetTextCount();
	PrintSuccess("Textos dormidos disponibles: " + std::to_string(count));
	
	// Mostrar ejemplos de cada categoría
	std::cout << "\nEjemplos por categoría:\n";
	
	const OracleDormantCategory categories[] = {
		OracleDormantCategory::DEATH_WARNINGS,
		OracleDormantCategory::DARKNESS_LORE,
		OracleDormantCategory::ANCIENT_WISDOM,
		OracleDormantCategory::CRYPTIC_PROPHECY,
		OracleDormantCategory::INFERNO_WHISPERS,
		OracleDormantCategory::FATE_ECHOES
	};
	
	const char* categoryNames[] = {
		"DEATH_WARNINGS",
		"DARKNESS_LORE",
		"ANCIENT_WISDOM",
		"CRYPTIC_PROPHECY",
		"INFERNO_WHISPERS",
		"FATE_ECHOES"
	};
	
	for (size_t i = 0; i < 6; ++i) {
		auto text = OracleDormantTexts::GetRandomText(categories[i]);
		if (text.has_value()) {
			std::cout << "\n📜 " << categoryNames[i] << ":\n";
			std::cout << "   \"" << text->substr(0, 80) << "...\"\n";
		}
	}
	
	return true;
}

bool Test3_PromptConstruction() {
	PrintSeparator("TEST 3: Construcción de Prompts");
	
	std::cout << "Construyendo prompt con textos dormidos...\n\n";
	
	std::string prompt = OraclePrompt::BuildPrompt(
		"Why do I keep dying?",
		"PLAYER_DEATH",
		PlayerState::ATTACK,
		"Level 5"
	);
	
	std::cout << "PROMPT GENERADO:\n";
	std::cout << "----------------------------------------\n";
	std::cout << prompt.substr(0, 500) << "...\n";
	std::cout << "----------------------------------------\n";
	
	// Verificar que el prompt contiene textos base
	if (prompt.find("EXAMPLE TEXTS") != std::string::npos) {
		PrintSuccess("Prompt contiene textos base obligatorios");
	} else {
		PrintError("Prompt NO contiene textos base");
		return false;
	}
	
	if (prompt.find("STRICT RULES") != std::string::npos) {
		PrintSuccess("Prompt contiene reglas estrictas");
	} else {
		PrintError("Prompt NO contiene reglas");
		return false;
	}
	
	return true;
}

bool Test4_OllamaQuery() {
	PrintSeparator("TEST 4: Query a Ollama (Asíncrono)");
	
	std::cout << "Construyendo prompt...\n";
	std::string prompt = OraclePrompt::BuildPrompt(
		"Why do I keep dying in the dungeons?",
		"PLAYER_DEATH",
		PlayerState::ATTACK,
		"Level 8"
	);
	
	std::cout << "Enviando query a Ollama...\n";
	
	bool responseReceived = false;
	std::string receivedResponse;
	
	OracleOllama::QueryAsync(prompt, [&](std::optional<std::string> response) {
		if (response.has_value()) {
			receivedResponse = *response;
		}
		responseReceived = true;
	});
	
	WaitForResponse(responseReceived, 10);
	
	if (!responseReceived) {
		PrintError("Timeout esperando respuesta de Ollama");
		return false;
	}
	
	if (receivedResponse.empty()) {
		PrintError("Respuesta vacía de Ollama");
		return false;
	}
	
	PrintSuccess("Respuesta recibida de Ollama");
	std::cout << "\n🔮 RESPUESTA DEL ORÁCULO:\n";
	std::cout << "----------------------------------------\n";
	std::cout << receivedResponse << "\n";
	std::cout << "----------------------------------------\n";
	
	return true;
}

bool Test5_ResponseValidation() {
	PrintSeparator("TEST 5: Validación de Respuestas");
	
	// Obtener textos base
	auto baseTexts = OracleDormantTexts::GetAllTexts(OracleDormantCategory::DEATH_WARNINGS);
	if (baseTexts.size() > 3) {
		baseTexts.resize(3);
	}
	
	std::cout << "Textos base para validación:\n";
	for (size_t i = 0; i < baseTexts.size(); ++i) {
		std::cout << (i + 1) << ". \"" << baseTexts[i].substr(0, 60) << "...\"\n";
	}
	
	// Construir prompt y obtener respuesta
	std::string prompt = OraclePrompt::BuildPrompt(
		"How can I survive longer?",
		"PLAYER_DEATH",
		PlayerState::ATTACK,
		"Level 10"
	);
	
	std::cout << "\nEnviando query a Ollama...\n";
	
	bool responseReceived = false;
	std::string receivedResponse;
	
	OracleOllama::QueryAsync(prompt, [&](std::optional<std::string> response) {
		if (response.has_value()) {
			receivedResponse = *response;
		}
		responseReceived = true;
	});
	
	WaitForResponse(responseReceived, 10);
	
	if (!responseReceived || receivedResponse.empty()) {
		PrintError("No se pudo obtener respuesta para validar");
		return false;
	}
	
	std::cout << "\n🔮 Respuesta a validar:\n";
	std::cout << "\"" << receivedResponse << "\"\n\n";
	
	// Validar respuesta
	ValidationResult validation = OracleValidator::ValidateResponse(
		receivedResponse,
		baseTexts
	);
	
	std::cout << "RESULTADO DE VALIDACIÓN:\n";
	std::cout << "----------------------------------------\n";
	std::cout << "Válida: " << (validation.isValid ? "✅ SÍ" : "❌ NO") << "\n";
	std::cout << "Similitud: " << std::fixed << std::setprecision(2) 
	          << (validation.similarity * 100) << "%\n";
	std::cout << "Razón: " << validation.reason << "\n";
	
	if (!validation.baseTextUsed.empty()) {
		std::cout << "Texto base detectado:\n";
		std::cout << "\"" << validation.baseTextUsed.substr(0, 80) << "...\"\n";
	}
	std::cout << "----------------------------------------\n";
	
	if (validation.isValid) {
		PrintSuccess("Respuesta VÁLIDA - Es micro-variación reconocible");
	} else {
		PrintError("Respuesta INVÁLIDA - " + validation.reason);
	}
	
	return validation.isValid;
}

bool Test6_CacheSystem() {
	PrintSeparator("TEST 6: Sistema de Cache");
	
	std::cout << "Inicializando cache...\n";
	OracleCache::Initialize();
	
	// Test 1: Cache miss
	std::cout << "\n1. Probando cache MISS...\n";
	auto cached1 = OracleCache::GetResponse("test question 1", "test base");
	
	if (!cached1.has_value()) {
		PrintSuccess("Cache MISS (esperado en primera consulta)");
	} else {
		PrintError("Cache HIT inesperado");
		return false;
	}
	
	// Test 2: Guardar en cache
	std::cout << "\n2. Guardando respuesta en cache...\n";
	OracleCache::SaveResponse(
		"test question 1",
		"test response 1",
		"test base",
		0.85f
	);
	PrintSuccess("Respuesta guardada en cache");
	
	// Test 3: Cache hit
	std::cout << "\n3. Probando cache HIT...\n";
	auto cached2 = OracleCache::GetResponse("test question 1", "test base");
	
	if (cached2.has_value()) {
		PrintSuccess("Cache HIT (respuesta recuperada)");
		std::cout << "   Respuesta: \"" << *cached2 << "\"\n";
	} else {
		PrintError("Cache MISS inesperado");
		return false;
	}
	
	// Test 4: Estadísticas
	std::cout << "\n4. Estadísticas del cache:\n";
	size_t totalEntries;
	uint32_t totalHits;
	float hitRate;
	
	OracleCache::GetStats(totalEntries, totalHits, hitRate);
	
	std::cout << "   Entradas totales: " << totalEntries << "\n";
	std::cout << "   Hits totales: " << totalHits << "\n";
	std::cout << "   Hit rate: " << std::fixed << std::setprecision(1) 
	          << (hitRate * 100) << "%\n";
	
	return true;
}

bool Test7_EndToEnd() {
	PrintSeparator("TEST 7: Flujo End-to-End Completo");
	
	std::cout << "Simulando flujo completo del Oráculo...\n\n";
	
	// Pregunta del jugador
	std::string question = "Why am I dying so much in the cathedral?";
	std::string event = "PLAYER_DEATH";
	std::string context = "Level 3";
	
	std::cout << "📝 Pregunta: \"" << question << "\"\n";
	std::cout << "⚔️  Evento: " << event << "\n";
	std::cout << "🏰 Contexto: " << context << "\n\n";
	
	// 1. Verificar cache
	std::cout << "1️⃣ Verificando cache...\n";
	auto cached = OracleCache::GetResponse(question, "");
	
	if (cached.has_value()) {
		std::cout << "   ✅ Cache HIT - Respuesta instantánea!\n";
		std::cout << "   🔮 \"" << *cached << "\"\n";
		return true;
	}
	
	std::cout << "   ℹ️  Cache MISS - Consultando Ollama\n\n";
	
	// 2. Obtener textos base
	std::cout << "2️⃣ Obteniendo textos dormidos...\n";
	OracleDormantCategory category = OracleDormantTexts::MapEventToCategory(event);
	auto baseTexts = OracleDormantTexts::GetAllTexts(category);
	
	if (baseTexts.size() > 3) {
		baseTexts.resize(3);
	}
	
	std::cout << "   ✅ " << baseTexts.size() << " textos base seleccionados\n\n";
	
	// 3. Construir prompt
	std::cout << "3️⃣ Construyendo prompt...\n";
	std::string prompt = OraclePrompt::BuildPrompt(
		question,
		event,
		PlayerState::ATTACK,
		context
	);
	std::cout << "   ✅ Prompt construido (" << prompt.length() << " chars)\n\n";
	
	// 4. Query a Ollama
	std::cout << "4️⃣ Consultando Ollama...\n";
	std::cout << "   🔮 El Oráculo medita tu pregunta...\n";
	
	bool responseReceived = false;
	std::string receivedResponse;
	
	auto startTime = std::chrono::high_resolution_clock::now();
	
	OracleOllama::QueryAsync(prompt, [&](std::optional<std::string> response) {
		if (response.has_value()) {
			receivedResponse = *response;
		}
		responseReceived = true;
	});
	
	WaitForResponse(responseReceived, 10);
	
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
	
	if (!responseReceived || receivedResponse.empty()) {
		PrintError("No se recibió respuesta de Ollama");
		return false;
	}
	
	std::cout << "   ✅ Respuesta recibida (" << duration.count() << "ms)\n\n";
	
	// 5. Validar respuesta
	std::cout << "5️⃣ Validando respuesta...\n";
	ValidationResult validation = OracleValidator::ValidateResponse(
		receivedResponse,
		baseTexts
	);
	
	std::cout << "   Similitud: " << std::fixed << std::setprecision(1) 
	          << (validation.similarity * 100) << "%\n";
	
	if (validation.isValid) {
		std::cout << "   ✅ Respuesta VÁLIDA\n\n";
		
		// 6. Mostrar respuesta
		std::cout << "6️⃣ Respuesta del Oráculo:\n";
		std::cout << "   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
		std::cout << "   🔮 " << receivedResponse << "\n";
		std::cout << "   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
		
		// 7. Guardar en cache
		std::cout << "7️⃣ Guardando en cache...\n";
		OracleCache::SaveResponse(
			question,
			receivedResponse,
			validation.baseTextUsed,
			validation.similarity
		);
		std::cout << "   ✅ Respuesta cacheada\n";
		
		return true;
	} else {
		std::cout << "   ❌ Respuesta INVÁLIDA: " << validation.reason << "\n";
		std::cout << "   ℹ️  Usando texto base como fallback\n\n";
		
		std::cout << "6️⃣ Respuesta del Oráculo (fallback):\n";
		std::cout << "   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
		std::cout << "   🔮 " << validation.baseTextUsed << "\n";
		std::cout << "   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
		
		return false;
	}
}

bool Test8_MultipleQuestions() {
	PrintSeparator("TEST 8: Múltiples Preguntas (Cache Performance)");
	
	struct TestQuestion {
		std::string question;
		std::string event;
		PlayerState state;
	};
	
	std::vector<TestQuestion> questions = {
		{"Why do I keep dying?", "PLAYER_DEATH", PlayerState::ATTACK},
		{"What should I do now?", "ENTERED_TOWN", PlayerState::FRIENDLY},
		{"Why do I keep dying?", "PLAYER_DEATH", PlayerState::ATTACK}, // Repetida
		{"How can I survive?", "PLAYER_DEATH", PlayerState::ATTACK},
		{"What should I do now?", "ENTERED_TOWN", PlayerState::FRIENDLY}, // Repetida
	};
	
	int cacheHits = 0;
	int cacheMisses = 0;
	
	for (size_t i = 0; i < questions.size(); ++i) {
		const auto& q = questions[i];
		
		std::cout << "\n" << (i + 1) << ". \"" << q.question << "\" (" << q.event << ")\n";
		
		// Verificar cache
		auto cached = OracleCache::GetResponse(q.question, "");
		
		if (cached.has_value()) {
			std::cout << "   ✅ Cache HIT (instantáneo)\n";
			std::cout << "   🔮 \"" << cached->substr(0, 60) << "...\"\n";
			cacheHits++;
		} else {
			std::cout << "   ℹ️  Cache MISS (consultando Ollama...)\n";
			
			// Construir prompt y consultar
			std::string prompt = OraclePrompt::BuildPrompt(
				q.question,
				q.event,
				q.state,
				""
			);
			
			bool received = false;
			std::string response;
			
			OracleOllama::QueryAsync(prompt, [&](std::optional<std::string> r) {
				if (r.has_value()) response = *r;
				received = true;
			});
			
			// Esperar respuesta
			for (int j = 0; j < 10 && !received; ++j) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
			
			if (received && !response.empty()) {
				std::cout << "   🔮 \"" << response.substr(0, 60) << "...\"\n";
				
				// Guardar en cache
				OracleCache::SaveResponse(q.question, response, "", 0.75f);
				cacheMisses++;
			}
		}
	}
	
	std::cout << "\n📊 ESTADÍSTICAS:\n";
	std::cout << "   Cache Hits: " << cacheHits << " (" 
	          << (cacheHits * 100 / questions.size()) << "%)\n";
	std::cout << "   Cache Misses: " << cacheMisses << " (" 
	          << (cacheMisses * 100 / questions.size()) << "%)\n";
	
	if (cacheHits >= 2) {
		PrintSuccess("Cache funcionando correctamente (preguntas repetidas cacheadas)");
		return true;
	} else {
		PrintError("Cache no está funcionando como esperado");
		return false;
	}
}

// ============================================================================
// 🔮 MAIN
// ============================================================================

int main() {
	std::cout << "🔮 TEST COMPLETO DEL SISTEMA DEL ORÁCULO\n";
	std::cout << "========================================\n\n";
	
	int passed = 0;
	int total = 0;
	
	// Test 1: Ollama disponible
	total++;
	if (Test1_OllamaAvailability()) {
		passed++;
	} else {
		std::cout << "\n⚠️  Ollama no está disponible. Los tests que requieren Ollama se saltarán.\n";
	}
	
	// Test 2: Textos dormidos
	total++;
	if (Test2_DormantTexts()) passed++;
	
	// Test 3: Construcción de prompts
	total++;
	if (Test3_PromptConstruction()) passed++;
	
	// Solo continuar con tests de Ollama si está disponible
	if (OracleOllama::IsAvailable()) {
		// Test 4: Query a Ollama
		total++;
		if (Test4_OllamaQuery()) passed++;
		
		// Test 5: Validación
		total++;
		if (Test5_ResponseValidation()) passed++;
		
		// Test 6: Cache
		total++;
		if (Test6_CacheSystem()) passed++;
		
		// Test 7: End-to-end
		total++;
		if (Test7_EndToEnd()) passed++;
		
		// Test 8: Múltiples preguntas
		total++;
		if (Test8_MultipleQuestions()) passed++;
	}
	
	// Resumen final
	PrintSeparator("RESUMEN FINAL");
	
	std::cout << "Tests ejecutados: " << total << "\n";
	std::cout << "Tests exitosos: " << passed << "\n";
	std::cout << "Tests fallidos: " << (total - passed) << "\n";
	std::cout << "Tasa de éxito: " << (passed * 100 / total) << "%\n\n";
	
	if (passed == total) {
		PrintSuccess("¡TODOS LOS TESTS PASARON! 🎉");
		std::cout << "\nEl sistema del Oráculo está funcionando perfectamente.\n";
		return 0;
	} else {
		PrintError("Algunos tests fallaron");
		std::cout << "\nRevisa los errores arriba para más detalles.\n";
		return 1;
	}
}
