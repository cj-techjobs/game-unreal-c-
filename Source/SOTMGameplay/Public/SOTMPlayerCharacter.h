// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "SOTMPlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class USOTMInteractionComponent;
class USOTMMovementPolicyComponent;
class UAbilitySystemComponent;
struct FInputActionValue;
struct FProgressionSnapshot;

/**
 * Physical avatar: movement, collision, animation, interaction
 * components. Holds no profile state; replaceable at respawn.
 *
 * Ported from BP_ThirdPersonCharacter, which bound Move/Look/Jump via
 * Enhanced Input directly in its event graph, plus a raw (non-Enhanced-Input)
 * LeftShift key toggling a "SprintSystem" Timeline that blended both
 * MaxWalkSpeed and camera FOV. The sprint key is promoted here to a proper
 * IA_Sprint Enhanced Input action (duplicated from IA_Jump) per the
 * architecture handbook's suggested input table, so it goes through the same
 * mapping-context/rebinding path as every other action instead of a
 * hardcoded key. The Timeline's exact eased curve keyframes could not be
 * recovered via available tooling (no editor access to inline Timeline
 * curve data), so the blend is reproduced as a straightforward FInterpTo
 * toward tunable target values instead -- functionally equivalent (smooth
 * speed/FOV transition on hold), but double-check the transition feel
 * against the original if it shipped with a deliberately tuned ease.
 *
 * Leaves camera/spring arm/spotlight/mesh/capsule alone -- those stay as
 * this Blueprint's existing artist-authored components rather than being
 * redeclared in C++, matching how ASOTMJumpscareEnemyCharacter finds its
 * Box component instead of recreating it.
 *
 * Also owns the InteractionComponent (handbook Part 2 section 1's
 * "interaction scanner") and binds IA_Interact to it -- this is new
 * infrastructure the original Blueprint never had, not a port of
 * existing behavior.
 *
 * Sprint speed is now resolved by USOTMMovementPolicyComponent (handbook
 * Part 2 section 2) rather than this class writing MaxWalkSpeed directly:
 * Start/StopSprint request a gait: the policy decides the actual effective
 * gait (it can refuse/cancel sprint for stamina or a restriction) and owns
 * the CharacterMovement speed blend. This class keeps only the camera FOV
 * blend, reading the policy's effective gait as its input -- FOV/camera is
 * presentation, not movement policy, per the same section's dimension
 * table.
 *
 * Holds no ASC of its own (handbook Part 2 section 1: "Its PlayerState
 * supplies the ASC") -- GetAbilitySystemComponent() forwards to the
 * possessing ASOTMPlayerState so the ASC/attributes survive this avatar
 * being destroyed and replaced at respawn. InitAbilityActorInfo is called
 * from both PossessedBy (server/standalone) and OnRep_PlayerState (client)
 * per standard GAS PlayerState-owned-ASC setup.
 *
 * InitializeAbilityActorInfo also (re)binds the PlayerState's
 * USOTMAttributeSetHealth::OnHealthDepleted to HandleHealthDepleted, which
 * forwards into UGameFlowSubsystem::TryAcceptDeath -- this is the handbook
 * Part 2 section 3.2 damage pipeline's remaining "lethal candidate ->
 * GameFlow acceptance" step. The binding lives here (not on the health set
 * itself) because HealthAttributeSet outlives this avatar across respawns
 * (it's PlayerState-owned) while this handler and this avatar do not, so
 * each new avatar needs to (re)establish its own binding rather than the
 * attribute set trying to own a reference back to whichever avatar is
 * currently possessing it. Closes the mechanical loop (fatal damage -> one
 * life consumed -> retry respawn or GameOver, real and testable via
 * UProgressionSubsystem::GetSnapshot) -- ASOTMChapterGameMode is what
 * actually consumes UGameFlowSubsystem::OnTerminalCandidateResolved to
 * drive the respawn itself (unpossess/destroy/RestartPlayer), since that's
 * a pawn-lifecycle authority decision, not this soon-to-be-replaced avatar's
 * to make about itself. A real game-over screen for the no-lives-left case
 * remains presentation-layer work with no owner yet.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASOTMPlayerCharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/**
	 * The ASC input adapter (handbook Part 2 section 1): finds the granted
	 * ability spec whose dynamic tags contain InputTag (stamped there by
	 * UProgressionSubsystem::ReconcileAbilityGrants from
	 * UAbilityDefinition::InputTag) and activates it. No-ops if nothing is
	 * currently granted for that slot. BlueprintCallable (not just a private
	 * input handler) so a PIE test driver can simulate an ability-slot press
	 * directly -- there's no way to simulate a real Enhanced Input key press
	 * through this project's MCP/DSL testing tools.
	 */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Abilities")
	void TryActivateAbilityByInputTag(FGameplayTag InputTag);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Input")
	TObjectPtr<UInputAction> InteractAction;

	/** Handbook Part 2 section 1's IA_AbilityPrimary -- "Slot intent resolved to owned ability ID" via Input.AbilityPrimary. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Input")
	TObjectPtr<UInputAction> AbilityPrimaryAction;

	/** Handbook Part 2 section 1's IA_AbilitySecondary -- same adapter, Input.AbilitySecondary. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Input")
	TObjectPtr<UInputAction> AbilitySecondaryAction;

	/** The handbook's Part 2 section 1 "interaction scanner" -- native infrastructure, not artist-authored, so it's a CreateDefaultSubobject here rather than left to Blueprint SCS. */
	UPROPERTY(VisibleAnywhere, Category = "SOTM|Interaction")
	TObjectPtr<USOTMInteractionComponent> InteractionComponent;

	/** Resolves requested gait/stance into CharacterMovement parameters (handbook Part 2 section 2). */
	UPROPERTY(VisibleAnywhere, Category = "SOTM|Movement")
	TObjectPtr<USOTMMovementPolicyComponent> MovementPolicyComponent;

	/** Target FollowCamera field of view while effectively sprinting. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Movement")
	float SprintFieldOfView = 100.0f;

	/** FInterpTo speed used to blend walk speed and FOV in/out of sprint. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Movement")
	float SprintTransitionSpeed = 6.0f;

	/** Seconds between AISense_Hearing noise reports while effectively sprinting (handbook Part 2 section 4.1: enemies hear, not just see). First tick of a new sprint always reports immediately. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|AI")
	float SprintNoiseInterval = 0.5f;

	/** Loudness passed to UAISense_Hearing::ReportNoiseEvent while sprinting. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|AI")
	float SprintNoiseLoudness = 1.5f;

	/** MaxRange passed to UAISense_Hearing::ReportNoiseEvent -- a hard cutoff independent of any listener's own HearingRange. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|AI")
	float SprintNoiseMaxRange = 2000.0f;

	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleSprintPressed();
	void HandleSprintReleased();
	void HandleCrouchPressed();
	void HandleCrouchReleased();
	void HandleInteract();
	void HandleAbilityPrimaryPressed();
	void HandleAbilitySecondaryPressed();

private:
	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> FollowCameraRef;

	float BaseFieldOfView = 0.0f;

	/** Counts up toward SprintNoiseInterval while sprinting; reset to the interval (so the next sprint start reports immediately) whenever not sprinting. */
	float TimeSinceLastSprintNoise = 0.0f;

	/** Binds the possessing PlayerState's ASC to this avatar, reconciles owned-upgrade ability grants against it, and (re)binds OnHealthDepleted. Idempotent -- safe to call from both PossessedBy and OnRep_PlayerState. */
	void InitializeAbilityActorInfo();

	UFUNCTION()
	void HandleProgressionChanged(const FProgressionSnapshot& Snapshot);

	/** Not a UFUNCTION: USOTMAttributeSetHealth::OnHealthDepleted is a native (non-dynamic) multicast delegate, bound via AddUObject rather than AddDynamic. */
	void HandleHealthDepleted(AActor* OwnerActor);

	/** Debug-only persistent on-screen status readout (wallet, owned upgrades, ability cooldown/active state, movement speed) -- no HUD widget exists yet. Called every Tick. */
	void PrintDebugStatus();
};
