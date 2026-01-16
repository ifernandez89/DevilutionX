/**
 * @file test_oracle_ollama.cpp
 * 
 * Test de compilación para el cliente Ollama
 */

#include "Source/oracle/oracle_ollama.h"
#include "Source/oracle/oracle_prompt.h"
#include "Source/oracle/oracle_events.h"
#include "Source/oracle/oracle_system.h"

#include <iostream>

using namespace devilution;

int main() {
	std::cout << "=== Test Oracle Ollama ===" << std::endl;
	
	// Test 1: Verificar disponibilidad
	std::cout << "1. Checking Ollama availability..." << std::endl;
	bool available = OracleOllama::IsAvailable();
	std::cout << "   Ollama available: " << (available ? "YES" : "NO") << std::endl;
	
	if (!available) {
		std::cout << "   Ollama not running. Start with: ollama serve" << std::endl;
		return 0;
	}
	
	// Test 2: Construir prompt
	std::cout << "\n2. Building prompt..." << std::endl;
	std::string prompt = OraclePrompt::BuildPrompt(
		"Why do I keep dying?",
		"PLAYER_DEATH",
		PlayerState::ATTACK,
		"Level 5"
	);
	std::cout << "   Prompt built successfully" << std::endl;
	
	// Test 3: Query asíncrono
	std::cout << "\n3. Querying Ollama (async)..." << std::endl;
	bool responseReceived = false;
	
	OracleOllama::QueryAsync(prompt, [&responseReceived](std::optional<std::string> response) {
		if (response.has_value()) {
			std::cout << "\n   Oracle response:" << std::endl;
			std::cout << "   🔮 " << *response << std::endl;
		} else {
			std::cout << "\n   No response from Ollama" << std::endl;
		}
		responseReceived = true;
	});
	
	// Esperar respuesta (máximo 10 segundos)
	std::cout << "   Waiting for response..." << std::endl;
	int waited = 0;
	while (!responseReceived && waited < 10000) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		waited += 100;
	}
	
	if (!responseReceived) {
		std::cout << "   Timeout waiting for response" << std::endl;
	}
	
	std::cout << "\n=== Test Complete ===" << std::endl;
	return 0;
}
